#include "sylar/sylar.h"

static auto g_logger = SYLAR_LOG_ROOT();


void run() {
    g_logger->setLevel(sylar::LogLevel::INFO);
    SYLAR_LOG_NAME("system")->setLevel(sylar::LogLevel::INFO);
    ptr<sylar::Address> addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if (!addr) {
        SYLAR_LOG_ERROR(g_logger) << "get address error";
        return;
    }
    ptr<sylar::http::HttpServer> http_server(new sylar::http::HttpServer);
    while (!http_server->bind(addr)) {
        SYLAR_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }
    http_server->start();
}

int main() {
    sylar::IOManager iom(1);
    iom.schedule(run);

    return 0;
}
