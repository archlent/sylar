/**
 * @file log.h
 * @author kangjinci (you@domain.com)
 * @brief  日志类
 * @version 0.1
 * @date 2021-06-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <cstdarg>
#include <functional>

#include "util.h"
#include "singleton.h"
#include "mutex.h"
#include "thread.h"

template<typename T>
using ptr = std::shared_ptr<T>;

namespace sylar {
    class LogLevel;
    class Logger;
    class LogEvent;
    class LogAppender;
    class LogFormatter;
}

namespace sylar {
    //日志级别
    class LogLevel {
    public:
        enum Level {
            UNKOWN = 0,                                                             // 未知级别           
            DEBUG,                                                                  // DEBUG 级别       
            INFO,                                                                   // INFO 级别      
            WARN,                                                                   // WARN 级别      
            ERROR,                                                                  // ERROR 级别       
            FATAL                                                                   // FATAL 级别   
        };
        static const char* ToString(LogLevel::Level level);
        static LogLevel::Level FromString(const std::string& s);
    };
}

namespace sylar {
    //日志事件
    class LogEvent {
    public:
        LogEvent(ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, 
            uint64_t threadId, uint32_t fiberId, uint64_t time, const std::string& thread_name) :
            m_file(file), m_line(line), m_elapse(elapse), m_threadId(threadId), m_fiberId(fiberId), m_time(time), 
            m_thread_name(thread_name), m_logger(logger), m_level(level) {

        }
        const char* getFile() const { return m_file; }                               //  返回文件名                                                
        auto getLine() const { return m_line; }                                      //  返回行号                          
        auto getElapse() const { return m_elapse; }                                  //  返回耗时                               
        auto getThreadId() const { return m_threadId;}                               //  返回线程ID                              
        auto getFiberId() const { return m_fiberId; }                                //  返回协程ID                                 
        auto getTime() const { return m_time; }                                      //  返回时间                           
        auto getLogger() const { return m_logger; }                                  //  返回日志器                                
        auto getLevel() const { return m_level; }                                    //  返回日志级别                             
        std::string getCountent() const { return m_ss.str(); }                       //  返回日志内容                                  
        std::stringstream& getSS() { return m_ss; }                                  //  返回日志内容字符串流
        const std::string& getThreadName() const { return m_thread_name; } 

        void format(const char* fmt, ...) {                                           // 格式化写入日志内容
            va_list al;
            va_start(al, fmt);
            format(fmt, al);
            va_end(al);
        }

        void format(const char* fmt, va_list al) {                                    // 格式化写入日志内容
            char* buf = nullptr;
            int len = vasprintf(&buf, fmt, al);
            if(len != -1) {
                m_ss << std::string(buf, len);
                free(buf);
            }
        }
    private:
        const char* m_file           = nullptr;                                      // 文件名
        int32_t m_line               = 0;                                            // 行号
        uint32_t m_elapse            = 0;                                            // 线程启动到现在的毫秒数
        uint64_t m_threadId          = 0;                                            // 线程ID
        uint32_t m_fiberId           = 0;                                            // 协程ID
        uint64_t m_time;                                                             // 时间戳
        std::string m_thread_name;
        std::stringstream m_ss;                                                      // 日志内容流        
        ptr<Logger> m_logger;                                                        // 日志器      
        LogLevel::Level m_level;                                                     // 日志等级          
    };

    // 日志事件包装器
    class LogEventWrap {
    public:
        LogEventWrap(ptr<LogEvent> e);
        ~LogEventWrap();
        std::stringstream& getSS();
        auto getEvent() const { return m_event; }
    private:
        ptr<LogEvent> m_event;
    };


    //日志器
    class Logger : public std::enable_shared_from_this<Logger> {
    friend class LoggerManager;
    public:
        using mutexType = sylar::Spinlock;
        Logger(const std::string& name = "root");
        void log(LogLevel::Level level, ptr<LogEvent> event);

        void debug(ptr<LogEvent> event);
        void info(ptr<LogEvent> event);
        void warn(ptr<LogEvent> event);
        void error(ptr<LogEvent> event);
        void fatal(ptr<LogEvent> event);

        void addAppender(ptr<LogAppender>);
        void delAppender(ptr<LogAppender>);
        void clearAppender() { 
            lock_guard<mutexType> lock(m_mutex); 
            m_appenders.clear(); 
        }
        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level level) { m_level = level; }
        void setFormatter(ptr<LogFormatter> val);
        void setFormatter(const std::string& fmt);
        auto getFormatter() const;

        const std::string& getName() const { return m_name; }
        std::string toYamlString();
    private:
        std::string m_name;                          //日志名称
        LogLevel::Level m_level;                    // 日志级别
        std::list<ptr<LogAppender>> m_appenders;    // appender集合
        ptr<LogFormatter> m_formatter;              // 日志格式器
        ptr<Logger> m_root;
        mutable mutexType m_mutex;
    };
    //日志格式器
    class LogFormatter {
    public:
        LogFormatter(const std::string& pattern);
        std::string format(ptr<Logger>, LogLevel::Level level, ptr<LogEvent> event);

        class FormatItem {
        public:
            virtual ~FormatItem() { }
            virtual void format(std::ostream& os, ptr<Logger>, LogLevel::Level level, ptr<LogEvent> event) = 0;
        };

        bool isError() const { return m_error; }
        const std::string& getPattern() const { return m_pattern; }
    private:
        void init();
    private:
        std::vector<ptr<FormatItem>> m_items;
        std::string m_pattern;
        bool m_error = false;
    };

    //日志输出地
    class LogAppender {
    friend class Logger;
    public:
        using mutexType = sylar::Spinlock;
        virtual ~LogAppender() { }

        virtual void log(ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) = 0;

        virtual std::string toYamlString() = 0;

        void setFormatter(ptr<LogFormatter> val) { 
            lock_guard<mutexType> lock(m_mutex);
            m_formatter = val;
            if (m_formatter) {
                m_hasFormatter = true;
            } else {
                m_hasFormatter = false;
            }
        }
        auto getFormatter() const { 
            lock_guard<mutexType> lock(m_mutex); 
            return m_formatter; 
        }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }
    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;
        ptr<LogFormatter> m_formatter;
        bool m_hasFormatter = false;
        mutable mutexType m_mutex;
    };

    // 输出到控制台的Appender
    class StdoutLogAppender : public LogAppender {
    public:
        void log(ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override;
        std::string toYamlString() override;
    private:

    };
    // 定义输出到文件的Appender
    class FileLogAppender : public LogAppender {
    public:
        FileLogAppender(const std::string& filename);
        void log(ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override;
        std::string toYamlString() override;

        //重新打开文件，文件打开成功返回true
        bool reopen();
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };

    // 日志器管理类
    class LoggerManager {
    friend class Singleton<LoggerManager>;
    public: 
        using mutexType = sylar::Spinlock;
        
        ptr<Logger> getLogger(const std::string& name);
        auto getRoot() const { return  m_root; }
        void init();
        std::string toYamlString();
    private:
        LoggerManager();
        std::unordered_map<std::string, ptr<Logger>> m_loggers;
        ptr<Logger> m_root;
        mutable mutexType m_mutex;
    };

    using LoggerMgr = Singleton<LoggerManager>;
}  // namespace sylar


#define SYLAR_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level)  \
        sylar::LogEventWrap(ptr<sylar::LogEvent>(new sylar::LogEvent(logger, level,\
                __FILE__, __LINE__, 0, sylar::GetThreadId(),\
                sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getSS() 

#define SYLAR_LOG_DEBUG(logger)   SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger)    SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO )
#define SYLAR_LOG_WARN(logger)    SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN )
#define SYLAR_LOG_ERROR(logger)   SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger)   SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, args...) \
    if (logger->getLevel() <= level) \
        sylar::LogEventWrap(ptr<sylar::LogEvent>(new sylar::LogEvent(logger, level, \
                __FILE__, __LINE__, 0, sylar::GetThreadId(), \
                sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getEvent()->format(fmt, ##args)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, args...)   SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, ##args)
#define SYLAR_LOG_FMT_INFO( logger, fmt, args...)   SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO , fmt, ##args)
#define SYLAR_LOG_FMT_WARN( logger, fmt, args...)   SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN , fmt, ##args)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, args...)   SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, ##args)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, args...)   SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, ##args)

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)