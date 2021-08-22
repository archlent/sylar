#include "tcp_server.h"
#include "config.h"


namespace sylar {
    static auto g_tcp_server_read_timeout = sylar::Config::Lookup("tcp_server.read_timeout", 
        (uint64_t)(60 * 1000 * 2), "tcp server read timeout");

    static auto g_logger = SYLAR_LOG_NAME("system");

    TcpServer::TcpServer(IOManager* worker, IOManager* accept_worker) : m_worker(worker), 
            m_acceptWorkder(accept_worker), m_recvTimeout(g_tcp_server_read_timeout->getValue()), 
            m_name("sylar/1.0.0"), m_isStop(true) {
    }

    TcpServer::~TcpServer() {
        for (auto& sock : m_socks) {
            sock->close();
        }
        m_socks.clear();
    }

    bool TcpServer::bind(ptr<Address> addr) {
        std::vector<ptr<Address>> addrs;
        std::vector<ptr<Address>> fails;
        addrs.push_back(addr);
        return bind(addrs, fails);
    }

    bool TcpServer::bind(const std::vector<ptr<Address>>& addrs, std::vector<ptr<Address>>& fails) {
        for (auto& addr : addrs) {
            auto sock = Socket::CreateTCP(addr);
            if (!sock->bind(addr)) {
                SYLAR_LOG_ERROR(g_logger) << "bind fail errno = " << errno
                 << " errstr = " << strerror(errno) << " addr = [" 
                 << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            if (!sock->listen()) {
                SYLAR_LOG_ERROR(g_logger) << "listen fail errno = " << errno
                 << "errstr = " << strerror(errno) << " addr = ["
                 << addr->toString() << "]";
                fails.push_back(addr);
                continue; 
            }
            m_socks.push_back(sock);
        }
        if (!fails.empty()) {
            m_socks.clear();
            return false;
        }
        for (auto& sock : m_socks) {
            SYLAR_LOG_INFO(g_logger) << "server bind success: " << *sock;
        }
        return true;
    }

    void TcpServer::startAccpet(ptr<Socket> sock) {
        while (!m_isStop) {
            auto client = sock->accept();
            if (client) {
                client->setRecvTimeout(m_recvTimeout);
                m_worker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
            } else {
                SYLAR_LOG_ERROR(g_logger) << "accept errno = " << errno << "errstr" 
                 << " errstr = " << strerror(errno);
            }
        }
    }

    bool TcpServer::start() {
        if (!m_isStop) {
            return true;
        }
        m_isStop = false;
        for (auto& sock : m_socks) {
            m_acceptWorkder->schedule(std::bind(&TcpServer::startAccpet, shared_from_this(), sock));
        }
        return true;
    }

    void TcpServer::stop() {
        m_isStop = true;
        auto self = shared_from_this();
        m_acceptWorkder->schedule([this, self](){
            for (auto& sock : m_socks) {
                sock->cancelAll();
                sock->close();
            }
            m_socks.clear();
        });
    }   

    void TcpServer::handleClient(ptr<Socket> client) {
        SYLAR_LOG_INFO(g_logger) << "handleCLient: " << *client;
    }

} // namespace sylar