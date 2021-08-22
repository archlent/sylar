#pragma once
#include "mutex.h"
#include "log.h"

#include <memory>
#include <functional>
#include <set>
#include <vector>

namespace sylar {
    class TimerManager; 
    class Timer : public std::enable_shared_from_this<Timer> {
    friend class TimerManager;
    public:
        bool cacel();
        bool refresh();
        bool reset(uint64_t ms, bool from_now);
    private:
        Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);
        Timer(uint64_t next);
    private:
        bool m_recurring;                   // 是否循环定时器
        uint64_t m_ms = 0;                  // 执行周期
        uint64_t m_next = 0;                // 精确的执行时间
        std::function<void()> m_cb;
        TimerManager* m_manager = nullptr;
    private:
        struct Comparator {
            bool operator()(const ptr<Timer>& lhs, const ptr<Timer>& rhs) const; 
        };
    };

    class TimerManager {
    friend class Timer;
    public:
        using RWMutexType = shared_mutex;
        TimerManager();
        virtual ~TimerManager();
        ptr<Timer> addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
        ptr<Timer> addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> week_cond, bool recurring = false);
        uint64_t getNextTimer();
        void listExpiriedTimer(std::vector<std::function<void()>>& cbs);
    protected:
        virtual void onTimerInsertedAtFront() = 0;
        void addTimer(ptr<Timer> val, WriteLock2& lock);
        bool detectClockRollover(uint64_t now_ms);
        bool hasTimer() const;
    private:
        mutable RWMutexType m_mutex;
        std::set<ptr<Timer>, Timer::Comparator> m_timers;
        bool m_tickle = false;
        uint64_t m_previouseTime;
    };
}