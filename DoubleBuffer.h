#pragma once

#include <mutex>
#include <memory>
#include <utility>

namespace CarRacing {
namespace Utils {

/**
 * @brief A double-buffering implementation for thread-safe data sharing
 * 
 * This class provides a double-buffering mechanism that allows one thread to write
 * to a back buffer while other threads can safely read from the front buffer.
 * The buffers can be swapped atomically.
 * 
 * @tparam T The type of data stored in the buffers
 */
template<typename T>
class DoubleBuffer {
public:
    /**
     * @brief Construct a new Double Buffer object
     * 
     * Initializes both front and back buffers with default-constructed T objects.
     */
    DoubleBuffer() 
        : frontBuffer_(std::make_unique<T>())
        , backBuffer_(std::make_unique<T>()) {
    }

    /**
     * @brief Construct a new Double Buffer object with initial data
     * 
     * @param initialData The initial data to copy to both buffers
     */
    explicit DoubleBuffer(const T& initialData) 
        : frontBuffer_(std::make_unique<T>(initialData))
        , backBuffer_(std::make_unique<T>(initialData)) {
    }

    // Prevent copying
    DoubleBuffer(const DoubleBuffer&) = delete;
    DoubleBuffer& operator=(const DoubleBuffer&) = delete;

    // Allow moving
    DoubleBuffer(DoubleBuffer&&) = default;
    DoubleBuffer& operator=(DoubleBuffer&&) = default;

    /**
     * @brief Swap the front and back buffers
     * 
     * This operation is atomic and thread-safe.
     */
    void Swap() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::swap(frontBuffer_, backBuffer_);
    }

    /**
     * @brief Get a const pointer to the front buffer for reading
     * 
     * This method is thread-safe and can be called concurrently with other methods.
     * 
     * @return const T* A pointer to the front buffer
     */
    const T* GetFrontBuffer() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return frontBuffer_.get();
    }

    /**
     * @brief Get a pointer to the back buffer for writing
     * 
     * This method is thread-safe, but the caller must ensure that only one thread
     * writes to the back buffer at a time.
     * 
     * @return T* A pointer to the back buffer
     */
    T* GetBackBufferForWriting() {
        std::lock_guard<std::mutex> lock(mutex_);
        return backBuffer_.get();
    }

    /**
     * @brief Apply a function to the back buffer
     * 
     * This method provides a convenient way to modify the back buffer
     * in a thread-safe manner.
     * 
     * @tparam Func The type of the function to apply
     * @param func The function to apply to the back buffer
     */
    template<typename Func>
    void ModifyBackBuffer(Func&& func) {
        std::lock_guard<std::mutex> lock(mutex_);
        func(*backBuffer_);
    }

private:
    std::unique_ptr<T> frontBuffer_;
    std::unique_ptr<T> backBuffer_;
    mutable std::mutex mutex_;
};

} // namespace Utils
} // namespace CarRacing