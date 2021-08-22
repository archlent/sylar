#include "sylar/log.h"
#include <iostream>
#include <thread>
#include <fstream>

int main() {
    ptr<sylar::Logger> logger(new sylar::Logger);
    logger->addAppender(ptr<sylar::LogAppender>(new sylar::StdoutLogAppender));

    //std::thread::id tid = std::this_thread::get_id();
    //std::cout << tid << std::endl;
    //uint64_t id = *(unsigned long*)(char*)&tid;
    //std::cout << "id = " << id << std::endl;

    // ptr<sylar::LogEvent> event(new sylar::LogEvent(__FILE__, __LINE__, 0, id, 2, time(0)));
    // event->getSS() << "Hello sylar log!";

    // logger->log(sylar::LogLevel::DEBUG, event);

    ptr<sylar::FileLogAppender> file_appender(new sylar::FileLogAppender("../txt/log.txt"));
    ptr<sylar::LogFormatter> fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::DEBUG);

    logger->addAppender(file_appender);

    SYLAR_LOG_INFO(logger) << "Test Macro";

    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_DEBUG(l) << "xxx";

    SYLAR_LOG_FMT_INFO(logger, "this is %s %d", "yyy", 250);

    std::cout << "Hello World!" << std::endl;

    return 0;
}