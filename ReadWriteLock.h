#pragma once

#include <shared_mutex>

namespace CarRacing {
namespace Utils {

/**
 * @brief A read-write lock implementation for concurrent access to shared resources
 * 
 * This class provides a mechanism for allowing multiple readers to access a resource
 * concurrently, while ensuring that writers have exclusive access. This is useful
 * for resources that are read frequently but written to infrequently.
 */
class ReadWriteLock {
public:
    ReadWriteLock() = default;
    ~ReadWriteLock() = default;

    // Prevent copying and moving
    ReadWriteLock(const ReadWriteLock&) = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;
    ReadWriteLock(ReadWriteLock&&) = delete;
    ReadWriteLock& operator=(ReadWriteLock&&) = delete;

    /**
     * @brief Acquire a shared (read) lock
     * 
     * Multiple threads can hold a read lock simultaneously.
     * This will block if a thread currently holds a write lock.
     */
    void LockRead() {
        mutex_.lock_shared();
    }

    /**
     * @brief Release a shared (read) lock
     */
    void UnlockRead() {
        mutex_.unlock_shared();
    }

    /**
     * @brief Acquire an exclusive (write) lock
     * 
     * Only one thread can hold a write lock at a time.
     * This will block if any thread currently holds a read or write lock.
     */
    void LockWrite() {
        mutex_.lock();
    }

    /**
     * @brief Release an exclusive (write) lock
     */
    void UnlockWrite() {
        mutex_.unlock();
    }

    /**
     * @brief RAII wrapper for read locks
     * 
     * This class provides a convenient RAII wrapper for read locks,
     * automatically releasing the lock when it goes out of scope.
     */
    class ReadLockGuard {
    public:
        explicit ReadLockGuard(ReadWriteLock& lock) : lock_(lock) {
            lock_.LockRead();
        }

        ~ReadLockGuard() {
            lock_.UnlockRead();
        }

        // Prevent copying and moving
        ReadLockGuard(const ReadLockGuard&) = delete;
        ReadLockGuard& operator=(const ReadLockGuard&) = delete;
        ReadLockGuard(ReadLockGuard&&) = delete;
        ReadLockGuard& operator=(ReadLockGuard&&) = delete;

    private:
        ReadWriteLock& lock_;
    };

    /**
     * @brief RAII wrapper for write locks
     * 
     * This class provides a convenient RAII wrapper for write locks,
     * automatically releasing the lock when it goes out of scope.
     */
    class WriteLockGuard {
    public:
        explicit WriteLockGuard(ReadWriteLock& lock) : lock_(lock) {
            lock_.LockWrite();
        }

        ~WriteLockGuard() {
            lock_.UnlockWrite();
        }

        // Prevent copying and moving
        WriteLockGuard(const WriteLockGuard&) = delete;
        WriteLockGuard& operator=(const WriteLockGuard&) = delete;
        WriteLockGuard(WriteLockGuard&&) = delete;
        WriteLockGuard& operator=(WriteLockGuard&&) = delete;

    private:
        ReadWriteLock& lock_;
    };

private:
    std::shared_mutex mutex_;
};

} // namespace Utils
} // namespace CarRacing