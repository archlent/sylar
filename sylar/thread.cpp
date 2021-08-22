#include "thread.h"

namespace sylar {
    static thread_local Thread* t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKOWN";

    Thread* Thread::GetThis() { return t_thread; }
    void Thread::SetThis(Thread* arg) { t_thread = arg; }
    const std::string& Thread::GetName() { return t_thread_name; }
    void Thread::SetName(const std::string& name) {
        if (name.empty()) {
            return;
        }
        if (t_thread) {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }
    void Thread::join() {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
    Thread::~Thread() {
        if (m_thread.joinable()) {
            m_thread.detach();
        }
    }
}