#include "timer.h"
#include "util.h"

namespace sylar {
    bool Timer::Comparator::operator()(const ptr<Timer>& lhs, const ptr<Timer>& rhs) const {
        if (!lhs && !rhs) {
            return false;
        }
        if (!lhs) {
            return true;
        }
        if (!rhs) {
            return false;
        }
        if (lhs->m_next != rhs->m_next) {
            return lhs->m_next < rhs->m_next;
        }
        return lhs.get() < rhs.get();
    }

    Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager)
        : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager) {
            m_next = sylar::GetCurrentMs() + m_ms;
    }

    Timer::Timer(uint64_t next) : m_next(next) { }

    bool Timer::cacel() {
        WriteLock2 lock(m_manager->m_mutex);
        if (m_cb) {
            m_cb = nullptr;
            auto it = m_manager->m_timers.find(shared_from_this());
            m_manager->m_timers.erase(it);
            return true;
        }
        return false;
    }

    bool Timer::refresh() {
        WriteLock2 lock(m_manager->m_mutex);
        if (!m_cb) {
            return false;
        }
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end()) {
            return false;
        }
        m_manager->m_timers.erase(it);
        m_next = m_ms + sylar::GetCurrentMs();
        m_manager->m_timers.emplace(shared_from_this());
        return true;
    }

    bool Timer::reset(uint64_t ms, bool from_now) {
        if (ms == m_ms && !from_now) {
            return true;
        }
        WriteLock2 lock(m_manager->m_mutex);
        if (!m_cb) {
            return false;
        }
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end()) {
            return false;
        }
        m_manager->m_timers.erase(it);
        uint64_t start = from_now ? sylar::GetCurrentMs() : m_next - m_ms;
        m_ms = ms;
        m_next = m_ms + start;
        m_manager->addTimer(shared_from_this(), lock);
        return true;
    }

    TimerManager::TimerManager() {
        m_previouseTime = sylar::GetCurrentMs();
        m_timers.clear();
    }
    TimerManager::~TimerManager() = default;

    void TimerManager::addTimer(ptr<Timer> val, WriteLock2& lock) {
        auto it = m_timers.insert(val).first;
        bool at_front = (it == m_timers.begin() && !m_tickle);
        if (at_front) {
            m_tickle = true;
        }
        lock.unlock();
        if (at_front) {
            onTimerInsertedAtFront();
        }
    }

    ptr<Timer> TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
        ptr<Timer> timer(new Timer(ms, cb, recurring, this));
        WriteLock2 lock(m_mutex);
        addTimer(timer, lock);
        return timer;
    }

    static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
        std::shared_ptr<void> tmp = weak_cond.lock();
        if (tmp) {
            cb();
        } 
    }

    ptr<Timer> TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring) {
        return addTimer(ms, [&](){ OnTimer(weak_cond, cb); }, recurring);
    }

    uint64_t TimerManager::getNextTimer() {
        ReadLock lock(m_mutex);
        m_tickle = false;
        if (m_timers.empty()) {
            return ~0ull;
        }
        const auto& next = *m_timers.begin();
        uint64_t now_ms = sylar::GetCurrentMs();
        if (now_ms >= next->m_next) {
            return 0;
        } else {
            return next->m_next - now_ms;
        }
    }
    
    void TimerManager::listExpiriedTimer(std::vector<std::function<void()>>& cbs) {
        uint64_t now_ms = sylar::GetCurrentMs();
        std::vector<ptr<Timer>> expired;
        {
            ReadLock lock(m_mutex);
            if (m_timers.empty()) {
                return;
            }
        }
        WriteLock2 lock(m_mutex);
        if (m_timers.empty()) {
            return;
        }
        bool rollover = detectClockRollover(now_ms);
        if (!rollover && ((*m_timers.begin())->m_next > now_ms)) {
            return;
        }

        ptr<Timer> now_timer(new Timer(now_ms));
        auto it = rollover ? m_timers.end() : m_timers.upper_bound(now_timer);
        expired.insert(expired.begin(), m_timers.begin(), it);
        m_timers.erase(m_timers.begin(), it);
        cbs.reserve(expired.size());

        for (auto& timer : expired) {
            cbs.emplace_back(timer->m_cb);
            if (timer->m_recurring) {
                timer->m_next = timer->m_ms + now_ms;
                m_timers.emplace(timer);
            } else {
                timer->m_cb = nullptr;
            }
        }
    }
    bool TimerManager::detectClockRollover(uint64_t now_ms) {
        bool rollover = false;
        if (now_ms < m_previouseTime && now_ms < (m_previouseTime - 60 * 60 * 1000)) {
            rollover = true;
        }
        m_previouseTime = now_ms;
        return rollover;
    }
    bool TimerManager::hasTimer() const {
        ReadLock lock(m_mutex);
        return !m_timers.empty();
    }
}