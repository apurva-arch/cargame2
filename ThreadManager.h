#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <memory>

#include "../utils/Logger.h"
#include "../utils/ThreadSafeQueue.h"

namespace CarRacing {
namespace Core {

/**
 * @brief Thread priority levels
 */
enum class ThreadPriority {
    Low,
    Normal,
    High,
    RealTime
};

/**
 * @brief Thread status
 */
enum class ThreadStatus {
    Idle,
    Running,
    Paused,
    Stopping,
    Stopped
};

/**
 * @brief A class for managing threads in the game engine
 * 
 * This class handles thread creation, management, and synchronization.
 * It provides facilities for creating worker threads, setting thread priorities,
 * and managing thread lifetimes.
 */
class ThreadManager {
public:
    /**
     * @brief Construct a new Thread Manager object
     */
    ThreadManager();
    
    /**
     * @brief Destroy the Thread Manager object
     * 
     * Stops all threads and cleans up resources.
     */
    ~ThreadManager();
    
    // Prevent copying and moving
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
    ThreadManager(ThreadManager&&) = delete;
    ThreadManager& operator=(ThreadManager&&) = delete;
    
    /**
     * @brief Initialize the thread manager
     * 
     * @param maxThreads The maximum number of threads to create (0 = use hardware concurrency)
     * @return true if initialization was successful, false otherwise
     */
    bool Initialize(unsigned int maxThreads = 0);
    
    /**
     * @brief Shutdown the thread manager
     * 
     * Stops all threads and cleans up resources.
     */
    void Shutdown();
    
    /**
     * @brief Create a new thread
     * 
     * @param name The name of the thread
     * @param function The function to run in the thread
     * @param priority The priority of the thread
     * @return true if the thread was created successfully, false otherwise
     */
    bool CreateThread(const std::string& name, std::function<void()> function, 
                     ThreadPriority priority = ThreadPriority::Normal);
    
    /**
     * @brief Stop a thread
     * 
     * @param name The name of the thread to stop
     * @return true if the thread was stopped successfully, false otherwise
     */
    bool StopThread(const std::string& name);
    
    /**
     * @brief Pause a thread
     * 
     * @param name The name of the thread to pause
     * @return true if the thread was paused successfully, false otherwise
     */
    bool PauseThread(const std::string& name);
    
    /**
     * @brief Resume a thread
     * 
     * @param name The name of the thread to resume
     * @return true if the thread was resumed successfully, false otherwise
     */
    bool ResumeThread(const std::string& name);
    
    /**
     * @brief Get the status of a thread
     * 
     * @param name The name of the thread
     * @return ThreadStatus The status of the thread
     */
    ThreadStatus GetThreadStatus(const std::string& name) const;
    
    /**
     * @brief Check if a thread exists
     * 
     * @param name The name of the thread
     * @return true if the thread exists, false otherwise
     */
    bool ThreadExists(const std::string& name) const;
    
    /**
     * @brief Get the number of active threads
     * 
     * @return unsigned int The number of active threads
     */
    unsigned int GetActiveThreadCount() const;
    
    /**
     * @brief Get the maximum number of threads
     * 
     * @return unsigned int The maximum number of threads
     */
    unsigned int GetMaxThreadCount() const;
    
    /**
     * @brief Create a worker thread pool
     * 
     * @param poolName The name of the thread pool
     * @param threadCount The number of threads in the pool
     * @param priority The priority of the threads
     * @return true if the pool was created successfully, false otherwise
     */
    bool CreateThreadPool(const std::string& poolName, unsigned int threadCount, 
                         ThreadPriority priority = ThreadPriority::Normal);
    
    /**
     * @brief Submit a task to a thread pool
     * 
     * @param poolName The name of the thread pool
     * @param task The task to submit
     * @return true if the task was submitted successfully, false otherwise
     */
    bool SubmitTask(const std::string& poolName, std::function<void()> task);
    
    /**
     * @brief Set the thread priority
     * 
     * @param name The name of the thread
     * @param priority The new priority
     * @return true if the priority was set successfully, false otherwise
     */
    bool SetThreadPriority(const std::string& name, ThreadPriority priority);
    
    /**
     * @brief Set the thread affinity (which CPU cores the thread can run on)
     * 
     * @param name The name of the thread
     * @param coreMask A bitmask of which cores the thread can run on
     * @return true if the affinity was set successfully, false otherwise
     */
    bool SetThreadAffinity(const std::string& name, uint64_t coreMask);

private:
    /**
     * @brief Set the native thread priority
     * 
     * @param thread The thread handle
     * @param priority The priority level
     * @return true if successful, false otherwise
     */
    bool SetNativeThreadPriority(std::thread& thread, ThreadPriority priority);
    
    /**
     * @brief Set the native thread affinity
     * 
     * @param thread The thread handle
     * @param coreMask A bitmask of which cores the thread can run on
     * @return true if successful, false otherwise
     */
    bool SetNativeThreadAffinity(std::thread& thread, uint64_t coreMask);
    
    /**
     * @brief Thread information structure
     */
    struct ThreadInfo {
        std::thread thread;
        std::string name;
        ThreadPriority priority;
        std::atomic<ThreadStatus> status;
        std::atomic<bool> shouldStop;
        std::mutex pauseMutex;
        std::condition_variable pauseCondition;
        bool isPaused;
    };
    
    /**
     * @brief Thread pool information structure
     */
    struct ThreadPoolInfo {
        std::vector<std::string> threadNames;
        Utils::ThreadSafeQueue<std::function<void()>> taskQueue;
    };
    
    /**
     * @brief Worker thread function
     * 
     * @param poolName The name of the thread pool
     * @param threadIndex The index of the thread in the pool
     */
    void WorkerThreadFunction(const std::string& poolName, unsigned int threadIndex);
    
    std::unordered_map<std::string, std::unique_ptr<ThreadInfo>> threads_;
    std::unordered_map<std::string, std::unique_ptr<ThreadPoolInfo>> threadPools_;
    mutable std::mutex mutex_;
    unsigned int maxThreads_;
    std::atomic<bool> initialized_;
    std::atomic<unsigned int> activeThreadCount_;
};

} // namespace Core
} // namespace CarRacing