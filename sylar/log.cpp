/**
 * @file log.cpp
 * @author kangjinci (you@domain.com)
 * @brief 日志类
 * @version 0.1
 * @date 2021-06-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "log.h"
#include <map>
#include <ctime>
#include "config.h"


namespace sylar {
    LogEventWrap::LogEventWrap(ptr<LogEvent> e) : m_event(e) {

    }
    LogEventWrap::~LogEventWrap() {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }
    std::stringstream& LogEventWrap::getSS() {
        return m_event->getSS();
    }
}  // namespace sylar

namespace sylar {
    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getCountent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getElapse();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem {
    public:
        FiberIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getFiberId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadNameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getThreadName();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getLogger()->getName();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem {
    public:
        DateTimeFormatItem(std::string format = "%Y-%m-%d %H:%M:%S") : m_format(format) {
            if (m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem {
    public:
        FilenameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        LineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        StringFormatItem(const std::string& str) : m_string(str) {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << m_string;
        }
    private:
        std::string m_string;
    };
    class TabFormatItem : public LogFormatter::FormatItem {
    public:
        TabFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) override {
            os << "\t";
            //os << "    ";
        }
    };
}  // namespace sylar

namespace sylar {
    // const char* LogLevel::ToString(LogLevel::Level level) {
    //     switch (level) {
    // #define XX(name)         
    //     case LogLevel::name: 
    //         return #name; 
    //         break;
    //     XX(DEBUG);
    //     XX(INFO);
    //     XX(WARN);
    //     XX(ERROR);
    //     XX(FATAL);
    // #undef XX
    //     default:
    //         return "UNKONW";
    //     }
    //     return "UNKONW";
    // }
    static const char* logLevel[6] = { "UNKONW","DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
    static std::unordered_map<std::string, LogLevel::Level> mp{{"UNKOWN", LogLevel::Level::UNKOWN}, {"DEBUG", LogLevel::Level::DEBUG}, 
            {"INFO",  LogLevel::Level::INFO}, {"WARN",    LogLevel::Level::WARN},  {"ERROR", LogLevel::Level::ERROR}, 
            {"FATAL", LogLevel::Level::FATAL}, {"unkown", LogLevel::Level::UNKOWN}, {"info", LogLevel::Level::INFO}, 
            {"debug", LogLevel::Level::DEBUG}, {"warn",   LogLevel::Level::WARN}, {"error",  LogLevel::Level::ERROR}};

    const char* LogLevel::ToString(LogLevel::Level level) {
        //static const char* logLevel[6] = { "UNKONW","DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
        if (level >= 0 && level < 6) {
            return logLevel[level];
        }
        return "UNKONW";
    }

    LogLevel::Level LogLevel::FromString(const std::string& s) {
        //static std::unordered_map<std::string, LogLevel::Level> mp{{"UNKOWN", UNKOWN}, {"DEBUG", DEBUG}, 
        //    {"INFO", INFO}, {"WARN", WARN}, {"ERROR", ERROR}, {"FATAL", FATAL}, {"unkown", UNKOWN}, {"info", INFO}, 
        //    {"debug", DEBUG}, {"warn", WARN}, {"error", ERROR}};
        auto it = mp.find(s);
        return it == mp.end() ? UNKOWN : it->second;
    }

    Logger::Logger(const std::string& name) : m_name(name), m_level(LogLevel::DEBUG) {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T<%f:%l>%T%m%n"));
    }
    void Logger::log(LogLevel::Level level, ptr<LogEvent> event) {
        if (level >= m_level) {
            if (!m_appenders.empty()) {
                lock_guard<mutexType> lock(m_mutex);
                auto self = shared_from_this();
                for (auto& i : m_appenders) {
                    i->log(self, level, event);
                }
            } else {
                if (m_root) {
                    m_root->log(level, event);
                }
            }
        }
    }
    void Logger::debug(ptr<LogEvent> event) {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(ptr<LogEvent> event) {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(ptr<LogEvent> event) {
        log(LogLevel::WARN, event);
    }
    void Logger::error(ptr<LogEvent> event) {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(ptr<LogEvent> event) {
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(ptr<LogAppender> appender) {
        lock_guard<mutexType> lock(m_mutex);
        if (!appender->getFormatter()) {
            lock_guard<mutexType> lock2(appender->m_mutex);
            appender->m_formatter = m_formatter;
        }
        m_appenders.push_back(appender);

    }
    void Logger::delAppender(ptr<LogAppender> appender) {
        lock_guard<mutexType> lock(m_mutex);
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
            if (*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    }
    void Logger::setFormatter(ptr<LogFormatter> val) {
        lock_guard<mutexType> lock(m_mutex);
        m_formatter = val;
        for (auto& i : m_appenders) {
            //lock_guard<mutexType> ll(i->m_mutex);
            if (!i->m_hasFormatter) {
                i->setFormatter(m_formatter);
            }
        }
    }
    void Logger::setFormatter(const std::string& fmt) {
        auto new_value = std::make_shared<LogFormatter>(fmt);
        if (new_value->isError()) {
            std::cout << "Logger setFormatter name = " << m_name << " value = " << fmt << "invalid foratter" << std::endl;
            return;
        }
        //m_formatter = new_value; 
        setFormatter(new_value);
    }
    inline auto Logger::getFormatter() const { 
        lock_guard<mutexType> lock(m_mutex);
        return m_formatter; 
    }

    std::string Logger::toYamlString() {
        lock_guard<mutexType> lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        if (m_level != LogLevel::UNKOWN) {
            node["level"] = LogLevel::ToString(m_level);
        }
        if (m_formatter) {
            node["fotmatter"] = m_formatter->getPattern();
        }
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
            node["appenders"].push_back(YAML::Load((*it)->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
}  // namespace sylar

namespace sylar {
    FileLogAppender::FileLogAppender(const std::string& filename) : m_filename(filename) {
        bool open = reopen();
        if (!open) {
            std::cout << "File cant't open!\n";
        }
    }
    void FileLogAppender::log(ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) {
        if (level >= m_level) {
            lock_guard<mutexType> lock(m_mutex);
            m_filestream << m_formatter->format(logger, level, event);
        }
    }
    bool FileLogAppender::reopen() {
        lock_guard<mutexType> lock(m_mutex);
        if (m_filestream) {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }
    std::string FileLogAppender::toYamlString() {
        lock_guard<mutexType> lock(m_mutex);
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if (m_level != LogLevel::UNKOWN) {
            node["level"] = LogLevel::ToString(m_level);
        }
        if (m_hasFormatter && m_formatter) {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void StdoutLogAppender::log(ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) {
        if (level >= m_level) {
            lock_guard<mutexType> lock(m_mutex);
            std::cout << m_formatter->format(logger, level, event);
        }
    }
    std::string StdoutLogAppender::toYamlString() {
        lock_guard<mutexType> lock(m_mutex);
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if (m_level != LogLevel::UNKOWN) {
            node["level"] = LogLevel::ToString(m_level);
        }
        if (m_hasFormatter && m_formatter) {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
}

namespace sylar {
    LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
        init();
    }
    // %xxx %xxx{xxx} %%
    void LogFormatter::init() {
        // str, format, type
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        size_t size = m_pattern.size();
        for (size_t i = 0; i < size; ++i) {
            if (m_pattern[i] != '%') {
                nstr.push_back(m_pattern[i]);
                continue;
            }
            if (i + 1 < size && m_pattern[i + 1] == '%') {
                nstr.push_back('%');
            }
            size_t j = i + 1;
            size_t fmt_status = 0, fmt_begin = 0;

            std::string str, fmt;
            for (; j < size; ++j) {
                if (!fmt_status && !isalpha(m_pattern[j]) && m_pattern[j] != '{' && m_pattern[j] != '}') {
                    str = m_pattern.substr(i + 1, j - i - 1);
                    break;
                }
                if (fmt_status == 0) {
                    if (m_pattern[j] == '{') {
                        str = m_pattern.substr(i + 1, j - i - 1);
                        fmt_status = 1;
                        fmt_begin = j;
                        continue;
                    }
                }
                else if (fmt_status == 1) {
                    if (m_pattern[j] == '}') {
                        fmt = m_pattern.substr(fmt_begin + 1, j - fmt_begin - 1);
                        fmt_status = 0;
                        j++;
                        break;
                    }
                }
                if (j == size - 1 && str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
            if (fmt_status == 0) {
                if (!nstr.empty()) {
                    vec.emplace_back(nstr, "", 0);
                    nstr.clear();
                }
                vec.emplace_back(str, fmt, 1);
                i = j - 1;
            }
            else if (fmt_status == 1) {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                m_error = true;
            }
        }
        if (!nstr.empty()) {
            vec.emplace_back(nstr, "", 0);
        }
        static std::unordered_map<std::string, std::function<ptr<FormatItem>(const std::string&)>> s_format_items = {
        #define XX(str, C) \
            {#str, [](const std::string& fmt) { return ptr<FormatItem>(new C(fmt)); } }
            XX(m, MessageFormatItem),           // %m 消息体
            XX(p, LevelFormatItem),             // %p level
            XX(r, ElapseFormatItem),            // %r 启动后的时间
            XX(c, NameFormatItem),              // c:日志名称
            XX(t, ThreadIdFormatItem),          // %t 线程id
            XX(n, NewLineFormatItem),           // %n 回车换行
            XX(d, DateTimeFormatItem),          // %d 时间
            XX(f, FilenameFormatItem),          // %f 文件名
            XX(l, LineFormatItem),              // %l 行号
            XX(T, TabFormatItem),               // %T tab
            XX(F, FiberIdFormatItem),           // %F 协程
            XX(N, ThreadNameFormatItem),        // %N 线程名称
        #undef XX
        };

        for (auto& i : vec) {
            if (std::get<2>(i) == 0) {
                m_items.push_back(ptr<FormatItem>(new StringFormatItem(std::get<0>(i))));
            }
            else {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end()) {
                    m_items.push_back(ptr<FormatItem>(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_error = true;
                }
                else {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
            //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
    }
    std::string LogFormatter::format(ptr<Logger> logger, LogLevel::Level level, ptr<LogEvent> event) {
        std::stringstream ss;
        for (auto& i : m_items) {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }
}  // namespace sylar


namespace sylar {
    LoggerManager::LoggerManager() {
        m_root.reset(new Logger);
        m_root->addAppender(ptr<LogAppender>(new StdoutLogAppender));

        m_loggers[m_root->m_name] = m_root;
        init();
    }
    
    ptr<Logger> LoggerManager::getLogger(const std::string& name) {
        lock_guard<mutexType> lock(m_mutex);
        auto it = m_loggers.find(name);
        if (it != m_loggers.end()) {
            return it->second;
        }
        auto logger = std::make_shared<Logger>(name);
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }

    void LoggerManager::init() { }

    std::string LoggerManager::toYamlString() {
        lock_guard<mutexType> lock(m_mutex);
        YAML::Node node;
        for (auto& [_, logger] : m_loggers) {
            node.push_back(YAML::Load(logger->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    struct LogAppenderDefine {
        int type = 0;  // 1 File   2 Stdout
        LogLevel::Level level = LogLevel::UNKOWN;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine& other) const {
            return type == other.type && level == other.level && formatter == other.formatter && file == other.file;
        }
    };

    struct LogDefine {
        std::string name;
        LogLevel::Level level = LogLevel::UNKOWN;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine& other) const {
            return name == other.name && level == other.level && formatter == other.formatter 
             && appenders == other.appenders;
        }
        bool operator<(const LogDefine& other) const {
            return name < other.name;
        }
    };


    template <>
    struct LexicalCast<std::string, LogDefine> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            if (!node["name"].IsDefined()) {
                std::cout << "log config error: name is null, " << node << std::endl;
                throw std::logic_error("log config name is null");
            }
            
            std::stringstream ss;
            LogDefine l;
            l.name = node["name"].as<std::string>();
            l.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>() : "");
            if (node["formatter"].IsDefined()) {
                l.formatter = node["formatter"].as<std::string>();
            }
            if (node["appenders"].IsDefined()) {
                for (size_t i = 0; i < node["appenders"].size(); ++i) {
                    auto a = node["appenders"][i];
                    if (!a["type"].IsDefined()) {
                        std::cout << "log config error appender type is null, " << a << std::endl;
                        continue;
                    }
                    LogAppenderDefine lad;
                    lad.level = LogLevel::FromString(a["level"].IsDefined() ? a["level"].as<std::string>() : "");
                    std::string type = a["type"].as<std::string>();
                    if (type == "FileLogAppender") {
                        lad.type = 1;
                        if (!a["file"].IsDefined()) {
                            std::cout << "log config error: fileappender file is null, " << a << std::endl;
                            continue;
                        }
                        lad.file = a["file"].as<std::string>();
                        if (a["formatter"].IsDefined()) {
                            lad.formatter = a["formatter"].as<std::string>();
                        }
                    } else if (type == "StdoutLogAppender") {
                        lad.type = 2;
                        if (a["formatter"].IsDefined()) {
                            lad.formatter = a["formatter"].as<std::string>();
                        } 
                    } else {
                        std::cout << "log config error: appender type is invalid, " << a << std::endl;
                        continue;
                    }
                    
                    l.appenders.push_back(lad);
                }
            }
            
            return l;
        }
    };
    template <>
    struct LexicalCast<LogDefine, std::string> {
        decltype(auto) operator() (const LogDefine& l) {
            YAML::Node node;
            node["name"] = l.name;
            if (l.level != LogLevel::UNKOWN) {
                node["level"] = LogLevel::ToString(l.level);
            }
            if (!l.formatter.empty()) {
                node["formatter"] = l.formatter;
            }
            for (auto& a : l.appenders) {
                YAML::Node na;
                if (a.type == 1) {
                    na["type"] = "FileLogAppender";
                    na["file"] = a.file;
                } else if (a.type == 2) {
                    na["type"] = "StdoutLogAppender";
                }

                if (a.level != LogLevel::UNKOWN) {
                    na["level"] = LogLevel::ToString(a.level);
                }
                if (!a.formatter.empty()) {
                    na["formatter"] = a.formatter;
                }

                node["appenders"].push_back(na);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    auto g_log_defines = Config::Lookup("logs", std::set<LogDefine>(), "logs config");

    struct LogIniter {
        LogIniter() {
            g_log_defines->addListener([](const std::set<LogDefine>& old_value, const std::set<LogDefine>& new_value) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
                for (auto& i : new_value) {
                    auto it = old_value.find(i);
                    ptr<Logger> logger;
                    if (it == old_value.end()) {          // 新增 logger
                        //logger.reset(new sylar::Logger(i.name));
                        logger = SYLAR_LOG_NAME(i.name);
                    } else {
                        if (!(i == *it)) {           // 修改 logger
                            logger = SYLAR_LOG_NAME(i.name);
                        }
                    }
                    logger->setLevel(i.level);
                    if (!i.formatter.empty()) {
                        logger->setFormatter(i.formatter);
                    }
                    logger->clearAppender();
                    for (auto& a : i.appenders) {
                        ptr<sylar::LogAppender> ap;
                        if (a.type == 1) {
                            ap.reset(new FileLogAppender(a.file));
                        } else if (a.type == 2) {
                            ap.reset(new StdoutLogAppender);
                        }
                        ap->setLevel(a.level);
                        if (!a.formatter.empty()) {
                            auto fmt = std::make_shared<LogFormatter>(a.formatter);
                            if (fmt->isError()) {
                                std::cout << "log.name = " << i.name << " appender type = " << a.type
                                    << " formatter = " << a.formatter << " is invalid" << std::endl;
                            } else {
                                ap->setFormatter(fmt);
                            }
                        }
                        logger->addAppender(ap);
                    } 
                }
                for (auto& i : old_value) {                // 删除 logger
                    auto it = new_value.find(i);
                    if (it == new_value.end()) {
                        auto logger = SYLAR_LOG_NAME(i.name);
                        logger->setLevel((LogLevel::Level)(100));
                        logger->clearAppender();
                    }
                }
            }); 
        }
    };

    static LogIniter __log_init;

}  // namespace sylar