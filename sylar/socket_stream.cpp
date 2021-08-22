#include "socket_stream.h"

namespace sylar {

SockStream::SockStream(ptr<Socket> sock, bool owner) 
    : m_socket(sock), m_owner(owner) {
}

SockStream::~SockStream() {
    if (m_owner && m_socket) {
        m_socket->close();
    }
}

bool SockStream::isConnected() const {
    return m_socket && m_socket->isConnected();
}

int SockStream::read(void* buffer, size_t length)  {
    if (!isConnected()) {
        return -1;
    }
    return m_socket->recv(buffer, length);
}

int SockStream::read(ptr<ByteArray> ba, size_t length)  {
    if (!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, length);
    int rt = m_socket->recv(&iovs[0], iovs.size());
    if (rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

int SockStream::write(const void* buffer, size_t length)  {
    if (!isConnected()) {
        return -1;
    }
    return m_socket->send(buffer, length);
}

int SockStream::write(ptr<ByteArray> ba, size_t length)  {
    if (!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getReadBuffers(iovs, length);
    int rt = m_socket->send(&iovs[0], iovs.size());
    if (rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

void SockStream::close()  {
    if (m_socket) {
        m_socket->close();
    }
}
}