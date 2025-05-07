#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <memory>

namespace CarRacing {
namespace Utils {

/**
 * @brief A thread-safe queue implementation for inter-thread communication
 * 
 * This queue provides thread-safe operations for pushing and popping elements,
 * with optional blocking behavior when attempting to pop from an empty queue.
 * 
 * @tparam T The type of elements stored in the queue
 */
template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // Prevent copying and moving
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    /**
     * @brief Push an item to the back of the queue
     * 
     * @param item The item to push
     */
    void Push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        cv_.notify_one();
    }

    /**
     * @brief Pop an item from the front of the queue
     * 
     * This method blocks until an item is available.
     * 
     * @return T The popped item
     */
    T Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    /**
     * @brief Try to pop an item from the front of the queue
     * 
     * This method does not block if the queue is empty.
     * 
     * @return std::optional<T> The popped item, or empty if the queue is empty
     */
    std::optional<T> TryPop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        
        std::optional<T> item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    /**
     * @brief Check if the queue is empty
     * 
     * @return true if the queue is empty, false otherwise
     */
    bool Empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Get the size of the queue
     * 
     * @return size_t The number of items in the queue
     */
    size_t Size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace Utils
} // namespace CarRacing