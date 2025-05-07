#include "ThreadManager.h"
#include "../utils/Profiler.h"

#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace CarRacing {
namespace Core {

ThreadManager::ThreadManager()
    : maxThreads_(0)
    , initialized_(false)
    , activeThreadCount_(0) {
}

ThreadManager::~ThreadManager() {
    Shutdown();
}

bool ThreadManager::Initialize(unsigned int maxThreads) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        Utils::Logger::GetInstance().Warning("ThreadManager", "Already initialized");
        return false;
    }
    
    // If maxThreads is 0, use hardware concurrency
    if (maxThreads == 0) {
        maxThreads_ = std::thread::hardware_concurrency();
        // Ensure at least 2 threads
        if (maxThreads_ < 2) {
            maxThreads_ = 2;
        }
    } else {
        maxThreads_ = maxThreads;
    }
    
    Utils::Logger::GetInstance().Info("ThreadManager", "Initialized with " + 
                                     std::to_string(maxThreads_) + " max threads");
    
    initialized_ = true;
    return true;
}

void ThreadManager::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    Utils::Logger::GetInstance().Info("ThreadManager", "Shutting down");
    
    // Stop all threads
    for (auto& pair : threads_) {
        auto& threadInfo = pair.second;
        
        if (threadInfo->status != ThreadStatus::Stopped) {
            threadInfo->shouldStop = true;
            
            // If the thread is paused, resume it so it can see the stop flag
            if (threadInfo->status == ThreadStatus::Paused) {
                {
                    std::lock_guard<std::mutex> pauseLock(threadInfo->pauseMutex);
                    threadInfo->isPaused = false;
                }
                threadInfo->pauseCondition.notify_one();
            }
            
            // Wait for the thread to finish
            if (threadInfo->thread.joinable()) {
                threadInfo->thread.join();
            }
            
            threadInfo->status = ThreadStatus::Stopped;
        }
    }
    
    threads_.clear();
    threadPools_.clear();
    activeThreadCount_ = 0;
    initialized_ = false;
}

bool ThreadManager::CreateThread(const std::string& name, std::function<void()> function, 
                               ThreadPriority priority) {
    PROFILE_FUNCTION();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Not initialized");
        return false;
    }
    
    if (ThreadExists(name)) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread already exists: " + name);
        return false;
    }
    
    if (threads_.size() >= maxThreads_) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Maximum thread count reached");
        return false;
    }
    
    auto threadInfo = std::make_unique<ThreadInfo>();
    threadInfo->name = name;
    threadInfo->priority = priority;
    threadInfo->status = ThreadStatus::Idle;
    threadInfo->shouldStop = false;
    threadInfo->isPaused = false;
    
    // Create a wrapper function that handles thread status and pausing
    auto threadFunction = [this, function, threadInfo = threadInfo.get()]() {
        threadInfo->status = ThreadStatus::Running;
        activeThreadCount_++;
        
        Utils::Logger::GetInstance().Info("ThreadManager", "Thread started: " + threadInfo->name);
        
        try {
            while (!threadInfo->shouldStop) {
                // Check if the thread should pause
                {
                    std::unique_lock<std::mutex> pauseLock(threadInfo->pauseMutex);
                    if (threadInfo->isPaused) {
                        threadInfo->status = ThreadStatus::Paused;
                        threadInfo->pauseCondition.wait(pauseLock, [threadInfo]() {
                            return !threadInfo->isPaused || threadInfo->shouldStop;
                        });
                        threadInfo->status = ThreadStatus::Running;
                        
                        // If we were woken up to stop, break out of the loop
                        if (threadInfo->shouldStop) {
                            break;
                        }
                    }
                }
                
                // Execute the thread function
                function();
                
                // If the function returns, we're done unless it's a worker thread
                // that should continue running
                if (threadInfo->name.find("Worker") == std::string::npos) {
                    break;
                }
            }
        } catch (const std::exception& e) {
            Utils::Logger::GetInstance().Error("ThreadManager", 
                "Exception in thread " + threadInfo->name + ": " + e.what());
        } catch (...) {
            Utils::Logger::GetInstance().Error("ThreadManager", 
                "Unknown exception in thread " + threadInfo->name);
        }
        
        threadInfo->status = ThreadStatus::Stopping;
        Utils::Logger::GetInstance().Info("ThreadManager", "Thread stopping: " + threadInfo->name);
        
        activeThreadCount_--;
        threadInfo->status = ThreadStatus::Stopped;
    };
    
    // Create the thread
    threadInfo->thread = std::thread(threadFunction);
    
    // Set the thread priority
    if (!SetNativeThreadPriority(threadInfo->thread, priority)) {
        Utils::Logger::GetInstance().Warning("ThreadManager", 
            "Failed to set thread priority for " + name);
    }
    
    threads_[name] = std::move(threadInfo);
    
    Utils::Logger::GetInstance().Info("ThreadManager", "Created thread: " + name);
    return true;
}

bool ThreadManager::StopThread(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threads_.find(name);
    if (it == threads_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread not found: " + name);
        return false;
    }
    
    auto& threadInfo = it->second;
    
    if (threadInfo->status == ThreadStatus::Stopped) {
        Utils::Logger::GetInstance().Warning("ThreadManager", "Thread already stopped: " + name);
        return true;
    }
    
    threadInfo->shouldStop = true;
    
    // If the thread is paused, resume it so it can see the stop flag
    if (threadInfo->status == ThreadStatus::Paused) {
        {
            std::lock_guard<std::mutex> pauseLock(threadInfo->pauseMutex);
            threadInfo->isPaused = false;
        }
        threadInfo->pauseCondition.notify_one();
    }
    
    // Wait for the thread to finish
    if (threadInfo->thread.joinable()) {
        threadInfo->thread.join();
    }
    
    threadInfo->status = ThreadStatus::Stopped;
    
    Utils::Logger::GetInstance().Info("ThreadManager", "Stopped thread: " + name);
    return true;
}

bool ThreadManager::PauseThread(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threads_.find(name);
    if (it == threads_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread not found: " + name);
        return false;
    }
    
    auto& threadInfo = it->second;
    
    if (threadInfo->status != ThreadStatus::Running) {
        Utils::Logger::GetInstance().Warning("ThreadManager", 
            "Thread not running, cannot pause: " + name);
        return false;
    }
    
    {
        std::lock_guard<std::mutex> pauseLock(threadInfo->pauseMutex);
        threadInfo->isPaused = true;
    }
    
    Utils::Logger::GetInstance().Info("ThreadManager", "Paused thread: " + name);
    return true;
}

bool ThreadManager::ResumeThread(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threads_.find(name);
    if (it == threads_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread not found: " + name);
        return false;
    }
    
    auto& threadInfo = it->second;
    
    if (threadInfo->status != ThreadStatus::Paused) {
        Utils::Logger::GetInstance().Warning("ThreadManager", 
            "Thread not paused, cannot resume: " + name);
        return false;
    }
    
    {
        std::lock_guard<std::mutex> pauseLock(threadInfo->pauseMutex);
        threadInfo->isPaused = false;
    }
    threadInfo->pauseCondition.notify_one();
    
    Utils::Logger::GetInstance().Info("ThreadManager", "Resumed thread: " + name);
    return true;
}

ThreadStatus ThreadManager::GetThreadStatus(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threads_.find(name);
    if (it == threads_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread not found: " + name);
        return ThreadStatus::Stopped;
    }
    
    return it->second->status;
}

bool ThreadManager::ThreadExists(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return threads_.find(name) != threads_.end();
}

unsigned int ThreadManager::GetActiveThreadCount() const {
    return activeThreadCount_;
}

unsigned int ThreadManager::GetMaxThreadCount() const {
    return maxThreads_;
}

bool ThreadManager::CreateThreadPool(const std::string& poolName, unsigned int threadCount, 
                                   ThreadPriority priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Not initialized");
        return false;
    }
    
    if (threadPools_.find(poolName) != threadPools_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread pool already exists: " + poolName);
        return false;
    }
    
    if (threads_.size() + threadCount > maxThreads_) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Maximum thread count would be exceeded");
        return false;
    }
    
    auto poolInfo = std::make_unique<ThreadPoolInfo>();
    
    // Create the worker threads
    for (unsigned int i = 0; i < threadCount; ++i) {
        std::string threadName = poolName + "_Worker" + std::to_string(i);
        
        if (!CreateThread(threadName, 
                         [this, poolName, i]() { WorkerThreadFunction(poolName, i); }, 
                         priority)) {
            // Clean up any threads we've already created
            for (const auto& name : poolInfo->threadNames) {
                StopThread(name);
            }
            
            return false;
        }
        
        poolInfo->threadNames.push_back(threadName);
    }
    
    threadPools_[poolName] = std::move(poolInfo);
    
    Utils::Logger::GetInstance().Info("ThreadManager", 
        "Created thread pool: " + poolName + " with " + std::to_string(threadCount) + " threads");
    
    return true;
}

bool ThreadManager::SubmitTask(const std::string& poolName, std::function<void()> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threadPools_.find(poolName);
    if (it == threadPools_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread pool not found: " + poolName);
        return false;
    }
    
    it->second->taskQueue.Push(std::move(task));
    return true;
}

bool ThreadManager::SetThreadPriority(const std::string& name, ThreadPriority priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threads_.find(name);
    if (it == threads_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread not found: " + name);
        return false;
    }
    
    auto& threadInfo = it->second;
    
    if (!SetNativeThreadPriority(threadInfo->thread, priority)) {
        Utils::Logger::GetInstance().Warning("ThreadManager", 
            "Failed to set thread priority for " + name);
        return false;
    }
    
    threadInfo->priority = priority;
    return true;
}

bool ThreadManager::SetThreadAffinity(const std::string& name, uint64_t coreMask) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threads_.find(name);
    if (it == threads_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", "Thread not found: " + name);
        return false;
    }
    
    auto& threadInfo = it->second;
    
    if (!SetNativeThreadAffinity(threadInfo->thread, coreMask)) {
        Utils::Logger::GetInstance().Warning("ThreadManager", 
            "Failed to set thread affinity for " + name);
        return false;
    }
    
    return true;
}

bool ThreadManager::SetNativeThreadPriority(std::thread& thread, ThreadPriority priority) {
#ifdef _WIN32
    // Windows implementation
    int nativePriority;
    
    switch (priority) {
        case ThreadPriority::Low:
            nativePriority = THREAD_PRIORITY_BELOW_NORMAL;
            break;
        case ThreadPriority::Normal:
            nativePriority = THREAD_PRIORITY_NORMAL;
            break;
        case ThreadPriority::High:
            nativePriority = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        case ThreadPriority::RealTime:
            nativePriority = THREAD_PRIORITY_HIGHEST;
            break;
        default:
            nativePriority = THREAD_PRIORITY_NORMAL;
            break;
    }
    
    return SetThreadPriority(thread.native_handle(), nativePriority) != 0;
#else
    // POSIX implementation
    int policy;
    struct sched_param param;
    
    pthread_getschedparam(thread.native_handle(), &policy, &param);
    
    switch (priority) {
        case ThreadPriority::Low:
            param.sched_priority = sched_get_priority_min(policy);
            break;
        case ThreadPriority::Normal:
            param.sched_priority = (sched_get_priority_min(policy) + 
                                   sched_get_priority_max(policy)) / 2;
            break;
        case ThreadPriority::High:
            param.sched_priority = sched_get_priority_max(policy) - 1;
            break;
        case ThreadPriority::RealTime:
            policy = SCHED_RR;
            param.sched_priority = sched_get_priority_max(policy);
            break;
        default:
            param.sched_priority = (sched_get_priority_min(policy) + 
                                   sched_get_priority_max(policy)) / 2;
            break;
    }
    
    return pthread_setschedparam(thread.native_handle(), policy, &param) == 0;
#endif
}

bool ThreadManager::SetNativeThreadAffinity(std::thread& thread, uint64_t coreMask) {
#ifdef _WIN32
    // Windows implementation
    DWORD_PTR mask = static_cast<DWORD_PTR>(coreMask);
    return SetThreadAffinityMask(thread.native_handle(), mask) != 0;
#else
    // POSIX implementation
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    for (int i = 0; i < 64; ++i) {
        if ((coreMask & (1ULL << i)) != 0) {
            CPU_SET(i, &cpuset);
        }
    }
    
    return pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset) == 0;
#endif
}

void ThreadManager::WorkerThreadFunction(const std::string& poolName, unsigned int threadIndex) {
    PROFILE_SCOPE(poolName + "_Worker" + std::to_string(threadIndex));
    
    auto poolIt = threadPools_.find(poolName);
    if (poolIt == threadPools_.end()) {
        Utils::Logger::GetInstance().Error("ThreadManager", 
            "Thread pool not found in worker function: " + poolName);
        return;
    }
    
    auto& taskQueue = poolIt->second->taskQueue;
    
    while (true) {
        // Check if we should stop
        auto threadIt = threads_.find(poolName + "_Worker" + std::to_string(threadIndex));
        if (threadIt == threads_.end() || threadIt->second->shouldStop) {
            break;
        }
        
        // Try to get a task from the queue
        auto task = taskQueue.TryPop();
        
        if (task) {
            // Execute the task
            try {
                PROFILE_SCOPE(poolName + "_Task");
                (*task)();
            } catch (const std::exception& e) {
                Utils::Logger::GetInstance().Error("ThreadManager", 
                    "Exception in worker task: " + std::string(e.what()));
            } catch (...) {
                Utils::Logger::GetInstance().Error("ThreadManager", 
                    "Unknown exception in worker task");
            }
        } else {
            // No task available, sleep for a short time
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

} // namespace Core
} // namespace CarRacing