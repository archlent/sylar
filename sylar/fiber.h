#pragma once
#include <memory>
#include <ucontext.h>
#include <functional>
#include "thread.h"

namespace sylar {
    class Fiber : public std::enable_shared_from_this<Fiber> {
    public:
        friend class Scheduler;
        enum State {
            INIT,              // 初始化状态
            HOLD,              // 暂停状态
            EXEC,              // 执行中状态   
            TERM,              // 结束状态
            READY,             // 可执行状态
            EXCEPT,           // 异常状态
        };
    private:
        Fiber();
    public:
        Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
        ~Fiber();

        void reset(std::function<void()> cb);                       // 重置协程函数 并重置协程状态
        void swapIn();                                              // 切换到当前协程执行
        void swapOut();                                             // 切换到后台执行
        void call();
        void back();

        uint64_t getId() const { return m_id; }
        State getState() const { return m_state; }
    public:
        static void SetThis(Fiber*);                                 // 设置当前协程
        static ptr<Fiber> GetThis();                                 // 返回当前协程
        static void YieldToReady(bool use_caller = false);                                 // 协程切换到后台，并设置为Ready状态
        static void YieldToHold(bool use_caller = false);                                  // 协程切换到后台，并设置为Hold状态
        static uint64_t TotalFibers();                             // 总协程数
        static void MainFunc();
        static void CallerMainFunc();
        static uint64_t GetFiberId();
    private:
        uint64_t m_id = 0;
        uint32_t m_stacksize = 0;
        State m_state = INIT;

        ucontext_t m_ctx;
        void* m_stack = nullptr;

        std::function<void()> m_cb;
    };
}