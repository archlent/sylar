#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"

static auto g_logger = SYLAR_LOG_ROOT();

void run() {
    auto addr = sylar::Address::LookupAny("0.0.0.0:8013");
    //auto addr2 = ptr<sylar::UnixAddress>(new sylar::UnixAddress("/tmp/unix_addr"));
    std::vector<ptr<sylar::Address>> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);
    ptr<sylar::TcpServer> tcp_server(new sylar::TcpServer);
    std::vector<ptr<sylar::Address>> fails;
    while (!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
}

int main() {
    sylar::IOManager iom(2);
    iom.schedule(run);

    return 0;
}