#include "sylar/socket.h"
#include "sylar/iomanager.h"

static auto g_logger = SYLAR_LOG_ROOT();

void test_socket() {
    auto addr = sylar::Address::LookupAnyIPAddress("www.baidu.com", AF_UNSPEC);
    //auto addr = sylar::Address::LookupAnyIpAddress("ipv6.test-ipv6.com", AF_UNSPEC);
    if (addr) {
        SYLAR_LOG_INFO(g_logger) << "get address: " << addr->toString();
    } else {
        SYLAR_LOG_ERROR(g_logger) << "get address fail";
        return;
    }
    ptr<sylar::Socket> sock = sylar::Socket::CreateTCP(addr);
    addr->setPort(80);
    if (!sock->connect(addr)) {
        SYLAR_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SYLAR_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if (rt <= 0) {
        SYLAR_LOG_ERROR(g_logger) << "send fail, rt = " << rt;
        return;
    } 
    std::string buf;
    buf.resize(4096);
    rt = sock->recv(buf.data(), buf.size());
    if (rt <= 0) {
        SYLAR_LOG_ERROR(g_logger) << "recv fail, rt = " << rt;
        return;
    }
    buf.resize(rt);
    SYLAR_LOG_INFO(g_logger) << buf;    
}

int main() {
    sylar::IOManager iom;
    iom.schedule(test_socket);

    return 0;
}