#include <iostream>
#include "sylar/http/http_connection.h"
#include "sylar/iomanager.h"

static auto g_logger = SYLAR_LOG_ROOT();

void run() {
    auto addr = sylar::Address::LookupAnyIPAddress("www.sylar.top:80");
    if (!addr) {
        SYLAR_LOG_INFO(g_logger) << "get addr error";
        return;
    }
    auto sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if (!rt) {
        SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
    }
    ptr<sylar::http::HttpConnection> conn(new sylar::http::HttpConnection(sock));
    ptr<sylar::http::HttpRequest> req(new sylar::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    SYLAR_LOG_INFO(g_logger) <<  "req: " << std::endl << *req;
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();    
    if (!rsp) {
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "rsp: " << std::endl << *rsp;
}

void run2() {
    //SYLAR_LOG_DEBUG(g_logger) << "------------------------------------------------";
    auto [result, response, error]  = *sylar::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 300);
    SYLAR_LOG_INFO(g_logger) << "result = " << (int)result << ", error = " << error << ", rsp = "
     << (response ? response->toString() : "");
}

int main() {
    sylar::IOManager iom(2);
    iom.schedule(run2);
}