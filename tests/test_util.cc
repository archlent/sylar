#include <iostream>
#include "sylar/util.h"
#include "sylar/log.h"
#include "sylar/macro.h"

auto g_logger = SYLAR_LOG_ROOT();

void test_assert() {
    SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10);
    //SYLAR_ASSERT(false);
    SYLAR_ASSERT2(false, "hello");
}

int main() {
    test_assert();

    return 0;
}