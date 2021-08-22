#include "http.h"

namespace sylar {
namespace http {

static std::unordered_map<std::string_view, HttpMethod> s_method_name = {
#define XX(num, name, string) {#name, HttpMethod::name}, 
    HTTP_METHOD_MAP(XX)
#undef XX
};

HttpMethod StringToHttpMethod(std::string_view m) {
#define XX(num, name, string)                           \
    if (strncmp(#name, m.data(), m.length()) == 0) {    \
        return HttpMethod::name;                        \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
 }

HttpMethod CharsToHttpMethod(std::string_view m) {
    auto it = s_method_name.find(m);
    return it == s_method_name.end() ? HttpMethod::INVALID_METHOD : it->second;
}

static const char* s_method_string[] = {
#define XX(num, name, string) #string, 
    HTTP_METHOD_MAP(XX)
#undef XX
};

const char* HttpMehodToString(const HttpMethod& m) {
    uint32_t idx = (uint32_t)m;
    if (idx >= 34) {
        return "<unknown>";
    }
    return s_method_string[idx];
}

const char* HtttpStatusToString(const HttpStatus& s) {
    switch (s) {
#define XX(code, name, msg)    \
        case HttpStatus::name: \
            return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
            return "<unkown>";
    }
}

HttpRequest::HttpRequest(uint8_t version, bool close) 
    : m_method(HttpMethod::GET), m_version(version), m_close(close),
      m_parserParamFlag(0), m_path("/") {
}

std::string HttpRequest::getHeader(const std::string& key, const std::string& def) const {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}

std::string HttpRequest::getParam(const std::string& key, const std::string& def) const {
    auto it = m_params.find(key);
    return it == m_params.end() ? def : it->second;
}

std::string HttpRequest::getCookie(const std::string& key, const std::string& def) const {
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? def : it->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val) {
    m_headers[key] = val;
}

void HttpRequest::setParam (const std::string& key, const std::string& val) {
    m_params[key] = val;
}

void HttpRequest::setCookie(const std::string& key, const std::string& val) {
    m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key) {
    m_headers.erase(key);
}

void HttpRequest::delParam (const std::string& key) {
    m_params.erase(key);
}

void HttpRequest::delCookie(const std::string& key) {
    m_cookies.erase(key);
}

bool HttpRequest::hasHeader(const std::string& key, std::string* val) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasParam (const std::string& key, std::string* val) {
    auto it = m_params.find(key);
    if (it == m_params.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasCookie(const std::string& key, std::string* val    ) {
    auto it = m_cookies.find(key);
    if (it == m_cookies.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

std::ostream& HttpRequest::dump(std::ostream& os) const {
    // GET/ uri HTTP/1.1
    // Host: www.baidu.com
    os << HttpMehodToString(m_method) << " " << m_path << (m_query.empty() ? "" : "?") << m_query
     << (m_fragment.empty() ? "" : "#") << m_fragment << " " << " HTTP/" << ((uint32_t)(m_version >> 4))
     << '.' << ((uint32_t)(m_version & 0x0F)) << "\r\n";
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    for (auto& i : m_headers) {
        if (strcasecmp(i.first.data(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

void HttpRequest::init() {
    std::string conn = getHeader("connection");
    if(!conn.empty()) {
        if(strcasecmp(conn.c_str(), "keep-alive") == 0) {
            m_close = false;
        } else {
            m_close = true;
        }
    }
}

HttpResponse::HttpResponse(uint8_t version, bool close) 
    : m_status(HttpStatus::OK), m_version(version), m_close(close) {

}


std::string HttpResponse::getHeader(const std::string& key, const std::string& def) const {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
    m_headers[key] = val;
}

void HttpResponse::delHeader(const std::string& key) {
    m_headers.erase(key);
}


std::ostream& HttpResponse::dump(std::ostream& os) const {
    os << "HTTP/" << (uint32_t)(m_version >> 4) << '.' << (uint32_t)(m_version & 0x0f) << " "
     << (uint32_t)m_status << " " << (m_reason.empty() ? HtttpStatusToString(m_status) : m_reason)
     << "\r\n";
    for (auto& i : m_headers) {
        if (strcasecmp(i.first.data(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    } 
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    } else {
        os << "\r\n\r\n";
    }
    return os;
}
}
}