#pragma once

#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>

#include "noncopyable.hpp"
#include "address.h"

namespace sylar {
    class Socket : public std::enable_shared_from_this<Socket>, noncopyable {
    public:
        enum Type {
            TCP = SOCK_STREAM, 
            UDP = SOCK_DGRAM
        };

        enum Family {
            IPv4 = AF_INET, 
            IPv6 = AF_INET6,
            Unix = AF_UNIX
        };

        static ptr<Socket> CreateTCP(ptr<Address> address);
        static ptr<Socket> CreateUDP(ptr<Address> address);

        static ptr<Socket> CreateTCPSocket();
        static ptr<Socket> CreateUDPSocket();

        static ptr<Socket> CreateTCPSocket6();
        static ptr<Socket> CreateUDPSocket6();

        static ptr<Socket> CreateUnixTCPSocket();
        static ptr<Socket> CreateUnixUDPSocket();

        Socket(int family, int type, int protocol = 0);
        ~Socket();

        int64_t getSendTimeout();
        void setSendTimeout(int64_t v);

        int64_t getRecvTimeout();
        void setRecvTimeout(int64_t v);

        bool getOption(int level, int option, void* result, socklen_t* len);
        template <typename T>
        bool getOption(int level, int option, T& result) {
            socklen_t length = sizeof(T);
            return getOption(level, option, &result, &length);
        }
        bool setOption(int level, int option, const void* result, socklen_t len);
        template <typename T>
        bool setOption(int level, int option, T& result) {
            return setOption(level, option, &result, sizeof(T));
        }
        ptr<Socket> accept();

        bool bind(const ptr<Address> addr);
        bool connect(const ptr<Address> addr, uint64_t timeout_ms = -1);
        bool listen(int backlog = SOMAXCONN);
        bool close();

        int send(const void* buffer, size_t length, int flags = 0);
        int send(const iovec* buffer, size_t length, int flags = 0);
        int sendTo(const void* buffer, size_t length, const ptr<Address> to, int flags = 0);
        int sendTo(const iovec* buffer, size_t length, const ptr<Address> to, int flags = 0);

        int recv(void* buffer, size_t length, int flags = 0);
        int recv(iovec* buffer, size_t length, int flags = 0);
        int recvFrom(void* buffer, size_t length, ptr<Address> from, int flags = 0);
        int recvFrom(iovec* buffer, size_t length, ptr<Address> from, int flags = 0);

        ptr<Address> getRemoteAddress();
        ptr<Address> getLocalAddress();

        int getFamily() const { return m_family; }
        int getType() const { return m_type; }
        int getProtocol() const { return m_protocol; }

        bool isConnected() const { return m_isConnected; }
        bool isValid() const;
        int getError();

        std::ostream& dump(std::ostream& os) const;
        std::string toString() const;
        int getSocket() const { return m_sock; }

        bool cancelRead();
        bool cancelWrite();
        bool cancelAccept();
        bool cancelAll();
    private:
        void initSock();
        void newSock();
        bool init(int sock);
    private:
        int m_sock;
        int m_family;
        int m_type;
        int m_protocol;
        bool m_isConnected;

        ptr<Address> m_localAddress;
        ptr<Address> m_remoteAddress;
    };

    inline std::ostream& operator<<(std::ostream& os, const Socket& sock) {
        return sock.dump(os);
    }
}
