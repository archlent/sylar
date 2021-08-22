/**
 * @file singleton.h
 * @author kangjinci (you@domain.com)
 * @brief 单例模式
 * @version 0.1
 * @date 2021-06-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <memory>

namespace sylar {
    template <typename T, class X = void, int N = 0>
    class Singleton {
    public:
        static T* GetInstance() {
            static T v;
            return &v;
        }

    };

    template <typename T, class X = void, int N = 0>
    class Singletonptr {
    public:
        static std::shared_ptr<T> GetInstance() {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };
}