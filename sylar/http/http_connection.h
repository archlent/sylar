#pragma once

#include "sylar/socket_stream.h"
#include "http.h"
#include "sylar/log.h"
#include "sylar/uri.h"

namespace sylar {
namespace http {

struct HttpResult {
    enum class Error {
        OK = 0,
        INVALID_URI = 1,
        INVALID_HOST = 2,
        CONNECT_FAIL = 3,
        SEND_CLOSE = 4,
        SEND_SOCKET_ERROR = 5,
        TIMEOUT = 6,
    };
    HttpResult(int _result, ptr<HttpResponse> _responce, const std::string& _error)
     : result(_result),  responce(_responce), error(_error) {}
    int result;
    ptr<HttpResponse> responce;
    std::string error;
};

class HttpConnection : public SockStream {
public:
    static ptr<HttpResult> DoGet(const std::string& uri, uint64_t timeout_ms, 
        const std::map<std::string, std::string>& headers = {}, 
        const std::string& body = "");

    static ptr<HttpResult> DoGet(ptr<Uri> uri, uint64_t timeout_ms,
        const std::map<std::string, std::string>& headers = {}, 
        const std::string& body = "");

    static ptr<HttpResult> DoPost(const std::string& uri, uint64_t timeout_ms, 
        const std::map<std::string, std::string>& headers = {}, 
        const std::string& body = "");

    static ptr<HttpResult> DoPost(ptr<Uri> uri, uint64_t timeout_ms, 
        const std::map<std::string, std::string>& headers = {}, 
        const std::string& body = "");

    static ptr<HttpResult> DoRequest(HttpMethod method, const std::string& uri, 
        uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, 
        const std::string& body = ""); 

    static ptr<HttpResult> DoRequest(HttpMethod method, ptr<Uri> uri, 
        uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, 
        const std::string& body = ""); 

    static ptr<HttpResult> DoRequest(ptr<HttpRequest> req, ptr<Uri> uri, 
        uint64_t timeout_ms); 

    HttpConnection(ptr<Socket> sock, bool owner = true);
    ptr<HttpResponse> recvResponse();
    int sendRequest(ptr<HttpRequest> rsp);            // 发送响应报文
};

class HttpConnectionPool {

private:
    std::string m_host;
    std::string m_vhost;
    int32_t port;
    int32_t m_maxSize;
    int32_t m_maxAliveTime;
    int32_t m_maxRequest;
};
}
}