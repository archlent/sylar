#pragma once

#include "stream.h"
#include "socket.h"

namespace sylar {
class SockStream : public Stream {
public:
    int read(void* buffer, size_t length) override;
    int read(ptr<ByteArray> ba, size_t length) override;
    int write(const void* buffer, size_t length) override;
    int write(ptr<ByteArray> ba, size_t length) override;
    void close() override;

    auto getSocket() const { return m_socket; }
    bool isConnected() const;
protected: 
    ptr<Socket> m_socket;
    bool m_owner;
public:     
    SockStream (ptr<Socket> sock, bool owner = true);
    ~SockStream();
};

}
