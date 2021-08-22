#pragma once

#include <memory>
#include "fiber.h"
#include "mutex.h"
#include "thread.h"
#include <queue>

namespace sylar {
    class Scheduler {
    public:
        using MutexType = mutex;
        // 构造函数     threads: 线程数量  use_caller: 是否使用当前调用线程  name: 协程调度器名称
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
        virtual ~Scheduler();
        const std::string& getName() const { return m_name; }
        static Scheduler* GetThis();
        static Fiber* GetMainFiber();

        void start();
        void stop();

        template <typename FiberOrcb>
        void schedule(FiberOrcb&& fc, int thread = -1) { 
            bool need_tickle = false;
            {
                lock_guard<MutexType> lock(m_mutex);
                need_tickle = scheduleNolock(std::forward<FiberOrcb>(fc), thread);
            }
            if (need_tickle) {
                tickle();
            }
        }  

        template <typename InputIterator>
        void schedule(InputIterator begin, InputIterator end) {
            bool need_tickle = false;
            {
                lock_guard<MutexType> lock(m_mutex);
                while (begin != end) {
                    need_tickle = scheduleNolock(&*begin, -1) || need_tickle;   
                    ++begin;
                }
            }
            if (need_tickle) {
                tickle();
            }
        }

    private:
        struct FiberAndThread {
            ptr<Fiber> fiber;                                                          // 协程             
            std::function<void()> cb;                                                  // 协程执行函数         
            int thread;                                                                // 线程id         

            FiberAndThread(const ptr<Fiber>& f, int thr) : fiber(f), thread(thr) { }
            FiberAndThread(const std::function<void()>& f, int thr) : cb(f), thread(thr) { }
            FiberAndThread(ptr<Fiber>&& f, int thr) noexcept : fiber(std::move(f)), thread(thr) { }
            FiberAndThread(std::function<void()>&& f, int thr) noexcept : cb(std::move(f)), thread(thr) { }
            FiberAndThread(ptr<Fiber>* f, int thr) : thread(thr) { fiber.swap(*f); }
            FiberAndThread(std::function<void()>* f, int thr) : thread(thr) { cb.swap(*f); }
            FiberAndThread() : thread(-1) { }
            void reset() {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };
    protected:
        virtual void tickle();                                                      //通知协程调度器有任务了
        void run();                                                                 //           
        virtual bool stopping();
        virtual void idle();
        void setThis();
        bool hasIdleThreads() const { return m_idleThreadCount > 0; }
    private: 
        template <typename FiberOrcb>
        bool scheduleNolock(FiberOrcb&& fc, int thread) {
            bool need_tickle = m_fibers.empty();
            FiberAndThread ft(std::forward<FiberOrcb>(fc), thread);
            if (ft.fiber || ft.cb) {
                m_fibers.push_back(std::move(ft));
            }
            return need_tickle; 
        }
    private:    
        MutexType m_mutex;                                                           // Mutex       
        std::vector<ptr<Thread>> m_threads;                                          // 线程池                                    
        std::list<FiberAndThread> m_fibers;                                         // 待执行的协程队列 
        ptr<Fiber> m_rootFiber;                                                      // user_caller为true时有效，调度协程                                       
        std::string m_name;                                                          //         
    protected:
        std::vector<int> m_threadIds;                                               // 线程id数组
        size_t m_threadCount = 0;                                                   // 线程数量
        std::atomic<size_t> m_activeThreadCount = {0};                              // 工作线程数量
        std::atomic<size_t> m_idleThreadCount = {0};                                // 空闲线程数量
        bool m_stopping = true;                                                     // 是否正在停止
        bool m_autoStop = false;                                                    // 是否自动停止
        int m_rootThread = 0;                                                       // 主线程id(use_caller)
    };
}  // namespace sylar