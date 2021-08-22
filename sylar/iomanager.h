#pragma once

#include "scheduler.h"
#include "timer.h"
namespace sylar {
    int setnonblocking(int fd);

    class IOManager : public Scheduler, public TimerManager {
    public:
        using RWMutexType = shared_mutex;
        enum Event {
            NONE   =  0x0,
            READ   =  0x1,
            WRITE  =  0x4
        };
    private:
        struct FdContext {
            using MutexType = mutex;
            struct EventContext {
                Scheduler* scheduler = nullptr;         // 事件执行的scheduler
                ptr<Fiber> fiber = nullptr;             // 事件协程
                std::function<void()> cb = nullptr;     // 事件的回调函数
            };
            EventContext read;                          // 读事件
            EventContext write;                         // 写事件
            int fd = 0;                                 // 事件关联的句柄
            Event events = NONE;                        // 已经注册的事件
            MutexType mutex_;                            // mutex
            EventContext& getContext(Event event);
            void resetContext(EventContext& ctx);
            void triggerEvent(Event event);
        };
    public:
        IOManager(size_t thread = 1, bool use_caller = true, const std::string& name = "");
        ~IOManager();
        // 1 success 0 retry -1 error
        int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
        bool delEvent(int fd, Event event); 
        bool cancelEvent(int fd, Event event);

        bool cancelAll(int fd);
        static IOManager* GetThis();
    protected:
        void tickle() override;
        bool stopping() override;
        void idle() override;
        void onTimerInsertedAtFront() override;

        bool stopping(uint64_t& timeout);

        void contextResize(size_t size);
    private:
        int m_epfd = 0;
        int m_tickleFds[2];
        std::atomic<size_t> m_pendingEventCount = {0};
        RWMutexType m_mutex;
        std::vector<FdContext*> m_fdContexts;   
    };
} // namespace sylar
