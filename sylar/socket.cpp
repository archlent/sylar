#include "socket.h"
#include "fd_manager.h"
#include "macro.h"
#include "hook.h"
namespace sylar {
    static auto g_logger = SYLAR_LOG_NAME("system");
    Socket::Socket(int family, int type, int protocol) : m_sock(-1), m_family(family), m_type(type),
        m_protocol(protocol), m_isConnected(false) {
        
    }

    ptr<Socket> Socket::CreateTCP(ptr<Address> address) {
        ptr<Socket> sock(new Socket(address->getFamily(), TCP));
        return sock;
    }

    ptr<Socket> Socket::CreateUDP(ptr<Address> address) {
        ptr<Socket> sock(new Socket(address->getFamily(), UDP));\
        return sock;
    }

    ptr<Socket> Socket::CreateTCPSocket() {
        ptr<Socket> sock(new Socket(IPv4, TCP));
        return sock;
    }

    ptr<Socket> Socket::CreateUDPSocket() {
        ptr<Socket> sock(new Socket(IPv4, UDP));
        return sock;

    }

    ptr<Socket> Socket::CreateTCPSocket6() {
        ptr<Socket> sock(new Socket(IPv6, TCP));
        return sock;
    }

    ptr<Socket> Socket::CreateUDPSocket6() {
        ptr<Socket> sock(new Socket(IPv6, UDP));
        return sock;
    }


    ptr<Socket> Socket::CreateUnixTCPSocket() {
        ptr<Socket> sock(new Socket(Unix, TCP));
        return sock;
    }

    ptr<Socket> Socket::CreateUnixUDPSocket() {
        ptr<Socket> sock(new Socket(Unix, UDP));
        return sock;
    }


    Socket::~Socket() {
        close();
    }

    int64_t Socket::getSendTimeout() {
        auto ctx = FdMsg::GetInstance()->get(m_sock);
        if (ctx) {
            return ctx->getTimeout(SO_SNDTIMEO);
        }
        return -1;
    }
    void Socket::setSendTimeout(int64_t v) {
        timeval tv{int(v / 1000), int(v % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
    }

    int64_t Socket::getRecvTimeout() {
        auto ctx = FdMsg::GetInstance()->get(m_sock);
        if (ctx) {
            return ctx->getTimeout(SO_RCVTIMEO);
        }
        return -1;
    }

    void Socket::setRecvTimeout(int64_t v) {
        timeval tv{int(v / 1000), int(v % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
    }

    bool Socket::getOption(int level, int option, void* result, socklen_t* len) {
        int rt = getsockopt(m_sock, level, option, result, len);
        if (rt) {
            SYLAR_LOG_DEBUG(g_logger) << "getOption sock = " << m_sock << " level = "
             << level << " option = " << option << " errno = " << errno << " errstr = " 
             << strerror(errno);
            return false;
        }
        return true;
    }

    bool Socket::setOption(int level, int option, const void* result, socklen_t len) {
        int rt = setsockopt(m_sock, level, option, result, len);
        if (rt) {
            SYLAR_LOG_DEBUG(g_logger) << "setOption sock = " << m_sock << " level = "
             << level << " option = " << option << " errno = " << errno << " errstr = " 
             << strerror(errno);
            return false;
        }
        return true;
    }

    ptr<Socket> Socket::accept() {
        ptr<Socket> sock(new Socket(m_family, m_type, m_protocol));
        int newsock = ::accept(m_sock, nullptr, nullptr);
        if (newsock == -1) {
            SYLAR_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno = " << errno 
             << " errstr = " << strerror(errno);
            return nullptr;
        }
        if (sock->init(newsock)) {
            return sock;
        }
        return nullptr;
    }

    bool Socket::init(int sock) {
        auto ctx = FdMsg::GetInstance()->get(sock);
        if (ctx && ctx->isSocket() && !ctx->isClose()) {
            m_sock = sock;
            m_isConnected = true;
            initSock();
            getLocalAddress();
            getRemoteAddress();
            return true;
        }
        return false;
    }

    bool Socket::bind(const ptr<Address> addr) {
        if (!isValid()) {
            newSock();
            if (SYLAR_UNLIKELY(!isValid())) {
                return false;
            }
        }
        if (SYLAR_UNLIKELY(addr->getFamily() != m_family)) {
            SYLAR_LOG_ERROR(g_logger) << "bind sock.family(" << m_family << ") addr.family" 
             << addr->getFamily() << ") not equal, addr = " << addr->toString();
            return false;
        }
        if (::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
            SYLAR_LOG_ERROR(g_logger) << "bind error = " << errno << " errstr = " << strerror(errno); 
            return false;
        }
        getLocalAddress();
        return true;
    }

    bool Socket::connect(const ptr<Address> addr, uint64_t timeout_ms) {
        if (!isValid()) {
            newSock();
            if (SYLAR_UNLIKELY(!isValid())) {
                return false;
            }
        }
        if (SYLAR_UNLIKELY(addr->getFamily() != m_family)) {
            SYLAR_LOG_ERROR(g_logger) << "connect sock.family(" << m_family << ") addr.family" 
             << addr->getFamily() << ") not equal, addr = " << addr->toString();
            return false;
        }
        if (timeout_ms == (uint64_t)-1) {
            if (::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
                SYLAR_LOG_ERROR(g_logger) << "sock = " << m_sock << " connect(" << addr->toString() 
                 << ") errno = " << errno << " errstr = " << strerror(errno);
                close();
                return false;
            } 
        } else {
            if (::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
                SYLAR_LOG_ERROR(g_logger) << "sock = " << m_sock << " connect(" << addr->toString() 
                << ") errno = " << errno << " errstr = " << strerror(errno);
                close();
                return false;
            }
        }
        m_isConnected = true;
        getLocalAddress();
        getRemoteAddress();
        return true;
    }

    bool Socket::listen(int backlog) {
        if (!isValid()) {
            SYLAR_LOG_ERROR(g_logger) << "liten error sock = -1";
            return false;
        }
        if (::listen(m_sock, backlog)) {
            SYLAR_LOG_ERROR(g_logger) << "listen error errno = " << errno << " errstr = " << strerror(errno);
            return false;   
        }
        return true;
    }

    bool Socket::close() {
        if (!m_isConnected && m_sock == -1) {
            return true;
        }
        m_isConnected = false;
        if (m_sock != -1) {
            ::close(m_sock);
            m_sock = -1;
        }
        return false;
    }

    int Socket::send(const void* buffer, size_t length, int flags) {
        if (isConnected()) {
            return ::send(m_sock, buffer, length, flags);
        }
        return -1;
    }

    int Socket::send(const iovec* buffer, size_t length, int flags) {
        if (isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffer;
            msg.msg_iovlen = length;
            return ::sendmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::sendTo(const void* buffer, size_t length, const ptr<Address> to, int flags) {
        if (isConnected()) {
            return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
        }
        return -1;
    }

    int Socket::sendTo(const iovec* buffer, size_t length, const ptr<Address> to, int flags) {
        if (isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffer;
            msg.msg_iovlen = length;
            msg.msg_name = to->getAddr();
            msg.msg_namelen = to->getAddrLen();
            return ::sendmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::recv(void* buffer, size_t length, int flags) {
        if (isConnected()) {
            return ::recv(m_sock, buffer, length, flags);
        }
        return -1;
    }

    int Socket::recv(iovec* buffer, size_t length, int flags) {
        if (isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = buffer;
            msg.msg_iovlen = length;
            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::recvFrom(void* buffer, size_t length, ptr<Address> from, int flags) {
        if (isConnected()) {
            socklen_t len = from->getAddrLen();
            return ::recvfrom(m_sock, buffer, length, flags, (sockaddr*)from->getAddr(), &len);
        }
        return -1;
    }

    int Socket::recvFrom(iovec* buffer, size_t length, ptr<Address> from, int flags) {
        if (isConnected()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffer;
            msg.msg_iovlen = length;
            msg.msg_name = from->getAddr();
            msg.msg_namelen = from->getAddrLen();
            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    ptr<Address> Socket::getRemoteAddress() {
        if (m_remoteAddress) {
            return m_remoteAddress;
        }
        ptr<Address> result;
        switch(m_family) {
            case AF_INET:
                result.reset(new IPv4Address());
                break;
            case AF_INET6:
                result.reset(new IPv6Address());
                break;
            case AF_UNIX:
                result.reset(new UnixAddress());
                break;
            default:
                result.reset(new UnknownAddress(m_family));
                break;
        }
        socklen_t addrlen = result->getAddrLen();
        if (getpeername(m_sock, result->getAddr(), &addrlen)) {
            SYLAR_LOG_ERROR(g_logger) << "getpeername error sock = " << m_sock
            << " errno = " << errno << " errstr = " << strerror(errno);
            return ptr<Address>(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX) {
            ptr<UnixAddress> addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        } 
        m_remoteAddress = result;
        return m_remoteAddress;
    }

    ptr<Address> Socket::getLocalAddress() {
        if (m_localAddress) {
            return m_localAddress;
        }
        ptr<Address> result;
        switch(m_family) {
            case AF_INET:
                result.reset(new IPv4Address());
                break;
            case AF_INET6:
                result.reset(new IPv6Address());
                break;
            case AF_UNIX:
                result.reset(new UnixAddress());
                break;
            default:
                result.reset(new UnknownAddress(m_family));
                break;
        }
        socklen_t addrlen = result->getAddrLen();
        if (getsockname(m_sock, result->getAddr(), &addrlen)) {
            SYLAR_LOG_ERROR(g_logger) << "getsockname error sock = " << m_sock
            << " errno = " << errno << " errstr = " << strerror(errno);
            return ptr<Address>(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX) {
            ptr<UnixAddress> addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        }
        m_localAddress = result;
        return m_localAddress;
    }

    bool Socket::isValid() const {
        return m_sock != -1;
    }

    int Socket::getError() {
        int error = 0;
        socklen_t len = sizeof(error);
        if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
            error = errno;
        }
        return error;
    }

    bool Socket::cancelRead() {
        return IOManager::GetThis()->cancelEvent(m_sock, sylar::IOManager::READ);
    }

    bool Socket::cancelWrite() {
        return IOManager::GetThis()->cancelEvent(m_sock, sylar::IOManager::WRITE);
    }

    bool Socket::cancelAccept() {
        return IOManager::GetThis()->cancelEvent(m_sock, sylar::IOManager::READ);
    }

    bool Socket::cancelAll() {
        return IOManager::GetThis()->cancelAll(m_sock);
    }

    void Socket::initSock() {
        int val = 1;
        setOption(SOL_SOCKET, SO_REUSEADDR, val);
        if (m_type == SOCK_STREAM) {
            setOption(IPPROTO_TCP, TCP_NODELAY, val);
        }
    }

    void Socket::newSock() {
        m_sock = socket(m_family, m_type, m_protocol);
        if (SYLAR_LIKELY(m_sock != -1)) {
            initSock();
        } else {
            SYLAR_LOG_ERROR(g_logger) << "socket(" << m_family << ", " << m_type << ", "
             << m_protocol << ") errno = " << errno << " errstr = " << strerror(errno);
        }
    }

    std::ostream& Socket::dump(std::ostream& os) const {
        os << "[Socket sock = " << m_sock << " is_connected = " << m_isConnected << " family = "
         << "type = " << m_type << " protocol = " << m_protocol;
        if (m_localAddress) {
            os << " local_address = " << m_localAddress->toString();
        }
        if (m_remoteAddress) {
            os << " remote_address = " << m_remoteAddress->toString();
        }
        return os;
    }

    std::string Socket::toString() const {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }

} // namespace sylar