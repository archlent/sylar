#include "sylar/fiber.h"
#include "sylar/log.h"

auto g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run in fiber begin";
    //sylar::Fiber::GetThis()->swapOut();
    sylar::Fiber::YieldToHold(true);
    SYLAR_LOG_INFO(g_logger) << "run in fiber end";
    //sylar::Fiber::YieldToHold();
}

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "main begin -1";
    {
    sylar::Fiber::GetThis();
    SYLAR_LOG_INFO(g_logger) << "main begin";
    ptr<sylar::Fiber> fiber(new sylar::Fiber(run_in_fiber, 0, true));    
    fiber->call();
    SYLAR_LOG_INFO(g_logger) << "main after call end";
    fiber->call();
    SYLAR_LOG_INFO(g_logger) << "main end";
    }
    SYLAR_LOG_INFO(g_logger) << "main after end2";
}


int main() {
    sylar::Thread::SetName("main");
    std::vector<ptr<sylar::Thread>> vec;
    vec.reserve(3);
    for (int i = 0; i < 3; ++i) {
        vec.emplace_back(new sylar::Thread("name_" + std::to_string(i), test_fiber));
    }
    for (auto& ptr : vec) {
        ptr->join();
    }

    return 0;
}