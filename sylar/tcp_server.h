#pragma once

#include "log.h"
#include "iomanager.h"
#include "socket.h"
#include "address.h"
#include "noncopyable.hpp"

namespace sylar {
class TcpServer : public std::enable_shared_from_this<TcpServer>, noncopyable {
public:
    TcpServer(IOManager* worker = IOManager::GetThis(), 
              IOManager* accept_worker = IOManager::GetThis());
    virtual ~TcpServer();
    virtual bool bind(ptr<Address> addr);  
    virtual bool bind(const std::vector<ptr<Address>>& addrs, std::vector<ptr<Address>>& fails);
    virtual bool start();
    virtual void stop();


    uint64_t getRecvTimeout() const { return m_recvTimeout; }
    std::string getName() { return m_name; }
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
    void setName(const std::string& name) { m_name = name; }  

    bool isStop() const { return m_isStop; }
protected:
    virtual void handleClient(ptr<Socket> client);
    virtual void startAccpet(ptr<Socket> sock);
private:
    std::vector<ptr<Socket>> m_socks;
    IOManager* m_worker;
    IOManager* m_acceptWorkder;
    uint64_t m_recvTimeout;
    std::string m_name;
    bool m_isStop;
};
}