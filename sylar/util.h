/**
 * @file util.h
 * @author kangjinci (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <unistd.h>
#include <sys/syscall.h>
#include <vector>
#include <execinfo.h>
#include <string>

namespace sylar {
    pid_t GetThreadId();
    __uint32_t GetFiberId();

    void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");
    uint64_t GetCurrentMs();
    uint64_t GetCurrentUs();
}
