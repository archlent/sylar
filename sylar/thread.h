#pragma once

#include <thread>
#include <functional>
#include <memory>
#include "log.h"
#include "util.h"
#include "noncopyable.hpp"
#include "mutex.h"

namespace sylar {
    class Thread;
    class Thread : noncopyable {
    public:
        template <typename fn, typename... Args>
        Thread(std::string name, fn&& func, Args&&... args) : m_name(name),
         m_thread([&](auto&&... arg) {
            SetThis(this);
            SetName(name);
            m_tid = sylar::GetThreadId();
            pthread_setname_np(pthread_self(), m_name.substr(0, 15).c_str());
            m_semapthore.notify();
            func(std::forward<decltype(arg)>(arg)...);
        }, std::forward<Args>(args)...) { m_semapthore.wait(); }

        pid_t getId() const { return m_tid; }
        const std::string& getName() const { return m_name; }
        static Thread* GetThis();
        static void SetThis(Thread*);
        static const std::string& GetName();
        static void SetName(const std::string& name);

        void join();
        ~Thread();

    private:
        pid_t m_tid = -1;             // 线程的真实id
        std::string m_name;           // 线程名称
        std::thread m_thread;         // 线程结构
        Semaphore m_semapthore;
    };
}  // namespace sylar