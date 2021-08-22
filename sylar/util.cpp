/**
 * @file util.cpp
 * @author kangjinci (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "util.h"
#include "log.h"
#include "fiber.h"
#include <execinfo.h>
#include <sys/time.h>

namespace sylar {
    static ptr<Logger> g_logger = SYLAR_LOG_NAME("system");



    pid_t GetThreadId() {
        return syscall(SYS_gettid);
    }
    __uint32_t GetFiberId() {
        return Fiber::GetFiberId();
    }

    void Backtrace(std::vector<std::string>& bt, int size, int skip) {
        auto array = (void**)malloc(size * sizeof(void*));
        size_t s = ::backtrace(array, size);
        char** strings = backtrace_symbols(array, s);
        if (strings == NULL) {
            SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols eroor";
            return;
        }
        for (size_t i = skip; i < s; ++i) {
            bt.push_back(strings[i]);
        }
        free(strings);
        free(array);
    }

    std::string BacktraceToString(int size, int skip, const std::string& prefix) {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for (size_t i = 0; i < bt.size(); ++i) {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }
    uint64_t GetCurrentMs() {
        timeval tv; 
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
    }
    uint64_t GetCurrentUs() {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
    }
}