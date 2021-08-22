#pragma once

#include "sylar/tcp_server.h"
#include "servlet.h"

namespace sylar {
namespace http {

class HttpServer : public sylar::TcpServer {
public:
    HttpServer(bool keepalive = false, IOManager* worker = IOManager::GetThis(), 
        IOManager* accept_worker = IOManager::GetThis());
    ptr<ServletDispatch> getServletDispatch() const { return m_dispatch; }
    void setServletDispatch(ptr<ServletDispatch> v) { m_dispatch = v; }
protected:
    void handleClient(ptr<Socket> client) override;
private:
    bool m_isKeepalive;
    ptr<ServletDispatch> m_dispatch;

};
}
}