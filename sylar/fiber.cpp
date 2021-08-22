#include "fiber.h"
#include "macro.h"
#include "config.h"
#include "scheduler.h"

#include <atomic>

namespace sylar {
    static ptr<Logger> g_logger = SYLAR_LOG_NAME("system");

    static std::atomic<uint64_t> s_fiber_id {0};
    static std::atomic<uint64_t> s_fiber_count {0};

    static thread_local Fiber* t_fiber = nullptr;                           // run fiber
    static thread_local ptr<Fiber> t_threadFiber = nullptr;                 // main fiber

    static auto g_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

    class MallocStackAllocator {
    public:
        static void* Alloc(size_t size) {
            return malloc(size);
        }
        static void Dealloc(void* vp, size_t size) {
            return free(vp);
        }
    };

    using StackAllocator = MallocStackAllocator;

    Fiber::Fiber() {
        m_state = EXEC;
        SetThis(this);

        if (getcontext(&m_ctx)) {
            SYLAR_ASSERT2(false, "getcontext");
        }
        ++s_fiber_count;
        SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
    }
    
    Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) 
            : m_id(++s_fiber_id), m_cb(cb)  {
        ++s_fiber_count;
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

        m_stack = StackAllocator::Alloc(m_stacksize);
        if (getcontext(&m_ctx)) {
            SYLAR_ASSERT2(false, "getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        if (!use_caller) {
            makecontext(&m_ctx, &Fiber::MainFunc, 0);
        } else {
            makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
        }
        SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
    }

    Fiber::~Fiber() {
        --s_fiber_count;
        if (m_stack) {
            SYLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
            StackAllocator::Dealloc(m_stack, m_stacksize);
        } else {
            SYLAR_ASSERT(!m_cb);
            SYLAR_ASSERT(m_state == EXEC);

            Fiber* cur = t_fiber;
            if (cur == this) {
                SetThis(nullptr);
            }
        }
        SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id << " total = " << s_fiber_count;
    }
    
    void Fiber::reset(std::function<void()> cb) {
        SYLAR_ASSERT(m_stack);
        SYLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
        m_cb = cb;
        if (getcontext(&m_ctx)) {
            SYLAR_ASSERT2(false, "getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = INIT;
    }

    void Fiber::call() {
        SetThis(this);
        m_state = EXEC;
        if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
            SYLAR_ASSERT2(false, "swapcontext");
        }
    }

    void Fiber::back() {
        SetThis(t_threadFiber.get());
        if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
            SYLAR_ASSERT2(false, "swapcontext");
        }
    }
    
    void Fiber::swapIn() {
        SetThis(this);
        SYLAR_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
            SYLAR_ASSERT2(false, "swapcontext");
        }
    }
   
    void Fiber::swapOut() {
        SetThis(Scheduler::GetMainFiber());
        if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
            SYLAR_ASSERT2(false, "swapcontext");
        }
    }

    void Fiber::SetThis(Fiber* f) {
        t_fiber = f;
    }

    ptr<Fiber> Fiber::GetThis() {
        if (t_fiber) {
            return t_fiber->shared_from_this();
        }
        ptr<Fiber> main_fiber(new Fiber);
        SYLAR_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }                       

    void Fiber::YieldToReady(bool use_caller) {
        auto cur = GetThis();
        cur->m_state = READY;
        if (!use_caller) {
            cur->swapOut();
        } else {
            cur->call();
        }
    }                   

    void Fiber::YieldToHold(bool use_caller) {
        auto cur = GetThis();
        SYLAR_ASSERT(cur->m_state == EXEC);
        // cur->m_state = HOLD;
        if (!use_caller) {
            cur->swapOut();
        } else {
            cur->back();
        }
    }                     

    uint64_t Fiber::TotalFibers() {
        return s_fiber_count;
    }           

    void Fiber::MainFunc() {
        auto&& cur = GetThis();
        SYLAR_ASSERT(cur);
        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception& ex) {
            cur->m_state = EXCEPT;
            SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << " fiber id = " << cur->getId() 
               << std::endl << sylar::BacktraceToString();
        } catch (...) {
            cur->m_state = EXCEPT;  
            SYLAR_LOG_ERROR(g_logger) << "Fiber Except: "<< " fiber id = " << cur->getId() 
               << std::endl << sylar::BacktraceToString();
        }
        auto raw_ptr = cur.get();
        //cur.reset();
        cur = nullptr;
        raw_ptr->swapOut();

        SYLAR_ASSERT2(false, "never reach fiber id = " + std::to_string(raw_ptr->getId()));
    }

    void Fiber::CallerMainFunc() {
        auto&& cur = GetThis();
        SYLAR_ASSERT(cur);
        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception& ex) {
            cur->m_state = EXCEPT;
            SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << " fiber id = " << cur->getId() 
               << std::endl << sylar::BacktraceToString();
        } catch (...) {
            cur->m_state = EXCEPT;  
            SYLAR_LOG_ERROR(g_logger) << "Fiber Except: "<< " fiber id = " << cur->getId() 
               << std::endl << sylar::BacktraceToString();
        }
        auto raw_ptr = cur.get();
        cur = nullptr;
        raw_ptr->back();

        SYLAR_ASSERT2(false, "never reach fiber id = " + std::to_string(raw_ptr->getId()));
    }

    uint64_t Fiber::GetFiberId() {
        if (t_fiber) {
            return t_fiber->getId();
        }
        return 0;
    }

}   // namespace sylar