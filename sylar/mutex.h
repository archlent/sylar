#pragma once
#include "noncopyable.hpp"

#include <mutex>
#include <shared_mutex>
#include <semaphore.h>
#include <atomic>
#include <boost/thread.hpp>

namespace sylar {
    class Semaphore : noncopyable {
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        void wait();
        void notify();
    private:
        Semaphore(Semaphore&&) = delete;
    private:
        sem_t m_semaphore;
    };

    using shared_mutex = std::shared_mutex;

    template <typename T>
    using lock_guard = std::lock_guard<T>;

    template <typename T>
    using unique_lock = std::unique_lock<T>;

    using ReadLock = std::shared_lock<shared_mutex>;
    using WriteLock = std::lock_guard<shared_mutex>;
    using WriteLock2 = std::unique_lock<shared_mutex>;
    using mutex = std::mutex;

    using upgrade_lock = boost::upgrade_lock<boost::shared_mutex>;
    using upgrade_to_unique_lock = boost::upgrade_to_unique_lock<boost::shared_mutex>;
    class NullMutex : noncopyable {             // 用于测试
    public:
        NullMutex() = default;
        ~NullMutex() = default;
        void lock() { }
        void unlock() { }
    };

    class Spinlock : noncopyable {
    public:
        Spinlock() {
            pthread_spin_init(&m_mutex, 0);
        }
        ~Spinlock() {
            pthread_spin_destroy(&m_mutex);
        }
        void lock() {
            pthread_spin_lock(&m_mutex);
        }
        void unlock() {
            pthread_spin_unlock(&m_mutex);
        }
    private:
        pthread_spinlock_t m_mutex;
    };

    class CASlock : noncopyable {
    public:
        CASlock() = default;
        ~CASlock() = default;
        void lock() {
            while (m_mutex.test_and_set(std::memory_order_acquire));
        }
        void unlock() {
            m_mutex.clear(std::memory_order_release);
        }
    private:
        std::atomic_flag m_mutex = ATOMIC_FLAG_INIT;
    };

} // namespace sylar