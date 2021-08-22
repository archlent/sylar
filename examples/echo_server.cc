#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/bytearray.h"

static auto g_logger = SYLAR_LOG_ROOT();

class EchoServer : public sylar::TcpServer {
public:
    EchoServer(int type) : m_type(type) {

    }

    void handleClient(ptr<sylar::Socket> client) override {
        SYLAR_LOG_INFO(g_logger) << "handleClient " << *client;
        ptr<sylar::ByteArray> ba(new sylar::ByteArray);
        while (true) {
            ba->clear();
            std::vector<iovec> iovs;
            ba->getWriteBuffers(iovs, 1024);
            int rt = client->recv(&iovs[0], iovs.size());
            if (rt == 0) {
                SYLAR_LOG_INFO(g_logger) << "client close" << *client;
                break;
            } else if (rt < 0) {
                SYLAR_LOG_INFO(g_logger) << "client error rt = " << rt
                 << " errno = " << errno << " errstr = " << strerror(errno);
                break;
            }
            ba->setPosition(ba->getPosition() + rt);
            ba->setPosition(0);
            if (m_type == 1)  {                                         // text
                std::cout << ba->toString();
            } else {
                std::cout << ba->toHexString();
            }
            std::cout.flush();
        }
    }
private:
    int m_type = 0;
};


int type = 1;

void run() {
    ptr<EchoServer> es(new EchoServer(type));
    auto addr = sylar::Address::LookupAny("0.0.0.0:8021"); 
    while (!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}


int main(int argc, char** argv) {
    if (argc > 1) {
        type = 2;
    }

    sylar::IOManager iom(2);
    iom.schedule(run);

    return 0;
}