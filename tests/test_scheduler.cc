#include "sylar/scheduler.h"
#include "sylar/hook.h"

static auto g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count = " << s_count;
    sleep_f(1);
    if (--s_count >= 0) {
        sylar::Scheduler::GetThis()->schedule(test_fiber, sylar::GetThreadId());
    }
}

int main() {
    SYLAR_LOG_INFO(g_logger) << "main";
    //sylar::Thread::SetName("test");
    sylar::Scheduler sc(3, true, "test");
    sc.start();
    sleep_f(2);
    SYLAR_LOG_INFO(g_logger) << "scheduler";
    sc.schedule(test_fiber);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";

    return 0;
}