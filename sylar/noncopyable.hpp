/**
 * @file noncopyable.hpp
 * @author kangjinci (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

namespace sylar {
    class noncopyable {
    public:
        noncopyable() = default;
        ~noncopyable() = default;
    private:
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    };
}