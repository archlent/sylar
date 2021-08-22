#include "sylar/thread.h"
#include <iostream>
#include <string_view>
#include <chrono>

std::mutex mu;
int count = 0;
sylar::shared_mutex s_mutex;

ptr<sylar::Logger> g_logger = SYLAR_LOG_ROOT();

struct Timer {
	std::chrono::time_point<std::chrono::system_clock> start, end;
	std::chrono::duration<float> duration;

	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		float ms = duration.count() * 1000.0f;
		std::cout << "Timer took " << ms << "ms" << std::endl;
	}
};

void fun1() {
    SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName() << " this.name: " << sylar::Thread::GetThis()->getName()
     << " id: " << sylar::GetThreadId() << " this.id: " << sylar::Thread::GetThis()->getId();
    for (int i = 0; i < 100000; ++i) {
        //sylar::WriteLock lock(s_mutex);
        //std::lock_guard<std::mutex> lock(mu);
        ++count;
    }
    //sleep(20);
}

void fun2() {
    for (int i = 0; i < 3; i++) 
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxx" << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << "xxxxxxxxxx" <<
         "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}

void fun3() {
    for (int i = 0; i < 3; i++)    
        SYLAR_LOG_INFO(g_logger) << "ooooooooooooooooooooooo" << "oooooooooooooooooooooooooooooooooooo" << "oooooooooo" << 
        "oooooooooooooooooooooooooooooooooooo";
}

void say(std::string_view s) {
    std::cout << s << std::endl;
}

struct A {
    void run(const std::string& s) {
        std::cout << s << std::endl;
    }
};

void add(int& count) {
    ++count;
}

void del(int* count) {
    (*count) --;
}

void Setname(const std::string& name) {
    sylar::Thread::SetName(name);
    SYLAR_LOG_INFO(g_logger) << "name = " << sylar::Thread::GetName();
}

int main() {
    Timer t;
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    std::vector<ptr<sylar::Thread>> thrs;
    //for (int i = 0; i < 5; ++i) {
    //    thrs.emplace_back(new sylar::Thread("name_" + std::to_string(i), fun1));
    //}
//
    //for (int i = 0; i < 5; ++i) {
    //    thrs[i]->join();
    //}

    for (int i = 0; i < 2; ++i) {
        thrs.emplace_back(new sylar::Thread("name_" + std::to_string(i * 2), fun2));
        thrs.emplace_back(new sylar::Thread("name_" + std::to_string(i * 2 + 1), fun3));
    }

    for (size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }

    SYLAR_LOG_INFO(g_logger) << "thread test end";

    SYLAR_LOG_INFO(g_logger) << "count = " << count;

    //sylar::Thread t("main", say, "Hello World!");
    //t.join();
    int count_ = 0;
    sylar::Thread t2("add", add, std::ref(count_));
    t2.join();
    SYLAR_LOG_INFO(g_logger) << "count_ = " << count_;
    
    A a;
    std::thread t3(&A::run, &a, "Hello std::thread");
    t3.join();
    sylar::Thread t4("main for", std::bind(&A::run, &a, "Hello sylar::Thread"));     // 只能使用std::bind,暂时没想到解决办法
    t4.join();
    
    sylar::Thread t5("del", del, &count_);
    SYLAR_LOG_INFO(g_logger) << "count_ = " << count_;
    t5.join();


    Setname("test set name");

    return 0;
}