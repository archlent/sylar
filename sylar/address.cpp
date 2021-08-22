#include "address.h"
#include "edian.h"
#include <sstream>
#include <netdb.h>
#include <string_view>
#include <ifaddrs.h>
namespace sylar {
    static auto g_logger = SYLAR_LOG_NAME("system");

    template <typename T>
    static T CreateMask(uint32_t bits) {
        return (1 << (sizeof(T) * 8 - bits)) - 1; 
    }

    template <typename T>
    static uint32_t CountBytes(T value) {
        uint32_t res = 0;
        while (value > 0) {
            value &= value - 1;
            ++res;
        }
        return res;
    }
    
    ptr<Address> Address::Create(const sockaddr* addr, socklen_t addrlen) {
        if (!addr) {
            return nullptr;
        }
        ptr<Address> result;
        switch(addr->sa_family) {
            case AF_INET:
                result.reset(new IPv4Address(*(const sockaddr_in*)addr));
                break;
            case AF_INET6:
                result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
                break;
            default:
                result.reset(new UnknownAddress(*addr));
                break;
        }
        return result;
    }

    bool Address::Lookup(std::vector<ptr<Address>>& result, const std::string& host, int family, int type, int protocol) {
        addrinfo hints, *res, *next;
        hints.ai_flags = 0;
        hints.ai_family = family;
        hints.ai_socktype = type;
        hints.ai_protocol = protocol;
        hints.ai_addrlen = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        std::string ip;                                             // ipv4 or ipv6 or 域名
        const char* service = NULL;                                // port

        if (!host.empty() && host[0] == '[') {
            const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
            if (endipv6) {
                if (*(endipv6 + 1) == ':') {
                    service = endipv6 + 2;
                }
                //ip = host.substr(1, endipv6 - host.c_str() - 1);
                ip = std::string_view(&host[1], endipv6 - host.c_str() - 1);
            }
        }

        if (ip.empty()) {
            service = (const char*)memchr(host.c_str(), ':', host.size());
            if (service) {
                if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                    ip = std::string_view(host.c_str(), service - host.c_str());
                    ++service;
                }
            }
        }

        if (ip.empty()) {
            ip = host;
        }
        int rt = getaddrinfo(ip.data(), service, &hints, &res);
        if (rt) {
            SYLAR_LOG_ERROR(g_logger) << "Address::Lookup getaddress(" << host << ", " << family 
             << ", " << type << ") err = " << rt << " errstr = " << strerror(rt);
            return false;
        }

        next = res;
        while (next) {
            result.push_back(Create(next->ai_addr, next->ai_addrlen));
            next = next->ai_next;
        }
        freeaddrinfo(res);
        return !result.empty();
    }

    std::optional<std::vector<ptr<Address>>> 
    Address::Lookup(std::string_view host, int family, int type, int protocol) {
        std::vector<ptr<Address>> result;
        addrinfo hints, *res, *next;
        hints.ai_flags = 0;
        hints.ai_family = family;
        hints.ai_socktype = type;
        hints.ai_protocol = protocol;
        hints.ai_addrlen = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        std::string ip;                                                // ipv4 or ipv6 or 域名
        const char* service = nullptr;                                // port

        if (!host.empty() && host[0] == '[') {
            const char* endipv6 = (const char*)memchr(host.data() + 1, ']', host.size() - 1);
            if (endipv6) {
                if (*(endipv6 + 1) == ':') {
                    service = endipv6 + 2;
                }
                ip = std::string_view(&host[1], endipv6 - host.data() - 1);
            }
        }

        if (ip.empty()) {
            service = (const char*)memchr(host.data(), ':', host.size());
            if (service) {
                if (!memchr(service + 1, ':', host.data() + host.size() - service - 1)) {
                    ip = std::string_view(host.data(), service - host.data());
                    ++service;
                }
            }
        }

        if (ip.empty()) {
            ip = host;
        }
        int rt = getaddrinfo(ip.data(), service, &hints, &res);
        if (rt) {
            SYLAR_LOG_ERROR(g_logger) << "Address::Lookup getaddress(" << host << ", " << family 
             << ", " << type << ") err = " << rt << " errstr = " << strerror(rt);
            return {};
        }

        next = res;
        while (next) {
            result.push_back(Create(next->ai_addr, next->ai_addrlen));
            next = next->ai_next;
        }
        freeaddrinfo(res);
        if (result.empty()) {
            return {};
        } 
        return result;
    }

    ptr<Address> Address::LookupAny(std::string_view host, int family, int type, int protocol) {
        auto&& result = Lookup(host, family, type, protocol);
        if (result) {
            return (*result)[0];
        }
        return nullptr;
    }

    ptr<IPAddress> Address::LookupAnyIPAddress(std::string_view host, int family, int type, int protocol) {
        auto&& result = Lookup(host, family, type, protocol);
        if (result) {
            for (auto& i : *result) {
                auto v = std::dynamic_pointer_cast<IPAddress>(i);
                if (v) {
                    return v;
                }
            }
        }
        return nullptr;
    }

    bool Address::GetInterfaceAddress(std::multimap<std::string_view, std::pair<ptr<Address>, uint32_t>>& result, 
                                        int family) {
        ifaddrs *next, *res;
        if (getifaddrs(&res) != 0) {
            SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress getifaddrs " << " err = " << errno 
             << " errstr = " << strerror(errno);
            return false;
        }

        try {
            for (next = res; next != nullptr; next = next->ifa_next) {
                ptr<Address> addr;
                uint32_t prefix_len = ~0u;
                if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                    continue;
                }
                switch(next->ifa_addr->sa_family) {
                    case AF_INET: {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        prefix_len = CountBytes(netmask);
                    }
                        break;
                    case AF_INET6: {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for (int i = 0; i < 16; i++) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                        break;
                    default:
                        break;
                }
                if (addr) {
                    result.emplace(next->ifa_name, std::make_pair(addr, prefix_len));
                }
            }
        } catch(...) {
            SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress exception";
            freeifaddrs(res);
            return false;
        }
        freeifaddrs(res);
        return true;
    }

    std::optional<std::multimap<std::string_view, std::pair<ptr<Address>, uint32_t>>>
    Address::GetInterfaceAddress(int family) {
        std::multimap<std::string_view, std::pair<ptr<Address>, uint32_t>> result;
        ifaddrs *next, *res;
        if (getifaddrs(&res) != 0) {
            SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress getifaddrs " << " err = " << errno 
             << " errstr = " << strerror(errno);
            return {};
        }

        try {
            for (next = res; next != nullptr; next = next->ifa_next) {
                ptr<Address> addr;
                uint32_t prefix_len = ~0u;
                if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                    continue;
                }
                switch(next->ifa_addr->sa_family) {
                    case AF_INET: {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        prefix_len = CountBytes(netmask);
                    }
                        break;
                    case AF_INET6: {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for (int i = 0; i < 16; i++) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                        break;
                    default:
                        break;
                }
                if (addr) {
                    result.emplace(next->ifa_name, std::make_pair(addr, prefix_len));
                }
            }
        } catch(...) {
            SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress exception";
            freeifaddrs(res);
            return {};
        }
        freeifaddrs(res);
        return result;
    }

    bool Address::GetInterfaceAddress(std::vector<std::pair<ptr<Address>, uint32_t>>& result, 
                std::string_view iface, int family) {
        if (iface.empty() || iface == "*") {
            if (family == AF_INET || family == AF_UNSPEC) {
                result.emplace_back(ptr<Address>(new IPv4Address()), 0u);
            }
            if (family == AF_INET6 || family == AF_UNSPEC) {
                result.emplace_back(new IPv6Address(), 0u);
            }
            return true;
        }
        std::multimap<std::string_view, std::pair<ptr<Address>, uint32_t>> res;
        if (!GetInterfaceAddress(res, family)) {
            return false;
        }
        auto its = res.equal_range(iface);
        for (auto it = its.first; it != its.second; ++it) {
            result.push_back(it->second);
        }
        return !result.empty();
    }

    std::optional<std::vector<std::pair<ptr<Address>, uint32_t>>>
    Address::GetInterfaceAddress(std::string_view iface, int family) {
        std::vector<std::pair<ptr<Address>, uint32_t>> result;
        if (iface.empty() || iface == "*") {
            if (family == AF_INET || family == AF_UNSPEC) {
                result.emplace_back(new IPv4Address(), 0u);
            }
            if (family == AF_INET6 || family == AF_UNSPEC) {
                result.emplace_back(new IPv6Address(), 0u);
            }
            return result;
        }
        auto res = GetInterfaceAddress(family);
        if (!res) {
            return {};
        }
        auto its = res->equal_range(iface);
        for (auto it = its.first; it != its.second; ++it) {
            result.push_back(it->second);
        }
        if (result.empty()) {
            return {};
        } 
        return result;
    }

    int Address::getFamily() const {
        return getAddr()->sa_family;
    }

    std::string Address::toString() const {
        std::stringstream ss;
        insert(ss);
        return ss.str();
    }

    bool Address::operator<(const Address& rhs) const {
        socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
        int result = memcmp(getAddr(), rhs.getAddr(), minlen);
        if (result < 0) {
            return true;
        } else if (result > 0) {
            return false;
        } else if (getAddrLen() < rhs.getAddrLen()) {
            return true;
        }
        return false;
    }

    bool Address::operator==(const Address& rhs) const {
        return getAddrLen() == rhs.getAddrLen() &&
             memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
    }

    bool Address::operator!=(const Address& rhs) const {
        return !(*this == rhs);
    }

    ptr<IPAddress> IPAddress::Create(const char* address, uint16_t port) {
        addrinfo hints, *res;
        bzero(&hints, sizeof(hints));

        hints.ai_flags = AI_NUMERICHOST;
        hints.ai_family = AF_UNSPEC;

        int rt = getaddrinfo(address, nullptr, &hints, &res);
        if (rt) {
            SYLAR_LOG_ERROR(g_logger) << "IPAddress::Create(" << address << ", " << port << ") rt = "
             << rt << " errno = " << strerror(errno);
            return nullptr;
        }
        try {
            ptr<IPAddress> result = std::dynamic_pointer_cast<IPAddress>(Address::Create(res->ai_addr, res->ai_addrlen));
            if (result) {
                result->setPort(port);
            }
            freeaddrinfo(res);
            return result;
        } catch (...) {
            freeaddrinfo(res);
            return nullptr;
        }
    }

    ptr<IPv4Address> IPv4Address::Create(const char* address, uint16_t port) {
        ptr<IPv4Address> rt(new IPv4Address);
        rt->m_addr.sin_family = AF_INET;
        rt->m_addr.sin_port = byteswapOnLittleEndian(port);
        int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
        if (result <= 0) {
            SYLAR_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", " << port << ") rt = "
             << result << " errno = " << strerror(errno);
            return nullptr;
        }
        return rt;
    }

    IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = byteswapOnLittleEndian(port);
        m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
    }

    const sockaddr* IPv4Address::getAddr() const {
        return (sockaddr*)&m_addr;
    }

    sockaddr* IPv4Address::getAddr() {
        return (sockaddr*)&m_addr;
    }

    socklen_t IPv4Address::getAddrLen() const {
        return sizeof(m_addr);
    }

    std::ostream& IPv4Address::insert(std::ostream& os) const {
        uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
        os << ((addr >> 24) & 0xff) << "." << ((addr >> 16) & 0xff) << "."
          << ((addr >> 8) & 0xff) << "." << (addr & 0xff);
        os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
        return os; 
    }

    ptr<IPAddress> IPv4Address::broadcastAddress(uint32_t prefix_len) {
        if (prefix_len > 32) {
            return nullptr;
        }
        sockaddr_in baddr(m_addr);
        //auto subNetMask = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        //baddr.sin_addr.s_addr = ((~subNetMask) | (baddr.sin_addr.s_addr & subNetMask));
        baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        return ptr<IPv4Address>(new IPv4Address(baddr));
    }

    ptr<IPAddress> IPv4Address::networdAddress(uint32_t prefix_len) {
        if (prefix_len > 32) {
            return nullptr;
        }
        sockaddr_in baddr(m_addr);
        baddr.sin_addr.s_addr &= ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        return ptr<IPv4Address>(new IPv4Address(baddr));
    }

    ptr<IPAddress> IPv4Address::subnetMask(uint32_t prefix_len) {
        if (prefix_len > 32) {
            return nullptr;
        }
        sockaddr_in subnet;
        bzero(&subnet, sizeof(subnet));
        subnet.sin_family = AF_INET;
        subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        return ptr<IPv4Address>(new IPv4Address(subnet));
    }

    uint32_t IPv4Address::getPort() const {
        return byteswapOnLittleEndian(m_addr.sin_port);
    }
    
    void IPv4Address::setPort(uint16_t v)  {
        m_addr.sin_port = byteswapOnLittleEndian(v);
    }

    ptr<IPv6Address> IPv6Address::Create(const char* address, uint16_t port) {
        ptr<IPv6Address> rt(new IPv6Address);
        rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
        int result = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr);
        if (result <= 0) {
            SYLAR_LOG_ERROR(g_logger) << "IPv6Address::Create(" << address << ", " << port << ") rt = "
             << result << " errno = " << strerror(errno);
            return nullptr;
        }
        return rt;
    }

    IPv6Address::IPv6Address() {
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
    }

    IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
        m_addr.sin6_port = byteswapOnLittleEndian(port);
        memcpy(&m_addr.sin6_addr.__in6_u, address, 16);
    }

    const sockaddr* IPv6Address::getAddr() const {
        return (sockaddr*)&m_addr;
    }

    sockaddr* IPv6Address::getAddr()  {
        return (sockaddr*)&m_addr;
    }

    socklen_t IPv6Address::getAddrLen() const {
        return sizeof(m_addr);
    }

    std::ostream& IPv6Address::insert(std::ostream& os) const {
        os << "[";
        uint16_t* addr = (uint16_t*)&m_addr.sin6_addr.s6_addr;
        os << std::hex;
        bool use_zero = false;
        for (size_t i = 0; i < 8; ++i) {
            if (addr[i] == 0 && !use_zero) {
                continue;
            }
            if (i && !addr[i - 1] && !use_zero) {
                os << ":";
                use_zero = true;
            }
            if (i > 0) {
                os << ":";
            }
            os << (int)byteswapOnLittleEndian(addr[i]);
        }

        if (!use_zero && !addr[7]) {
            os << "::";
        }

        os << std::dec << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
        return os;
    }

    ptr<IPAddress> IPv6Address::broadcastAddress(uint32_t prefix_len) {
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_family = AF_INET6;
        baddr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
        for (int i = prefix_len / 8 + 1; i < 16; ++i) {
            baddr.sin6_addr.s6_addr[i] = 0xff;
        }
        return ptr<IPv6Address>(new IPv6Address(baddr));
    }

    ptr<IPAddress> IPv6Address::networdAddress(uint32_t prefix_len) {
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_family = AF_INET6;
        baddr.sin6_addr.s6_addr[prefix_len / 8] &= ~CreateMask<uint8_t>(prefix_len % 8);
        for (int i = prefix_len / 8 + 1; i < 16; ++i) {
            baddr.sin6_addr.s6_addr[i] = 0x00;
        }
        return ptr<IPv6Address>(new IPv6Address(baddr));
    }

    ptr<IPAddress> IPv6Address::subnetMask(uint32_t prefix_len) {
        sockaddr_in6 subnet;
        bzero(&subnet, sizeof(subnet));
        subnet.sin6_family = AF_INET6;
        subnet.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);
        for (uint32_t i = 0; i < prefix_len / 8; ++i) {
            subnet.sin6_addr.s6_addr[i] = 0xff;
        }
        return ptr<IPv6Address>(new IPv6Address(subnet));
    }

    uint32_t IPv6Address::getPort() const {
        return byteswapOnLittleEndian(m_addr.sin6_port);
    }
    
    void IPv6Address::setPort(uint16_t v)  {
        m_addr.sin6_port = byteswapOnLittleEndian(v);
    }

    static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

    UnixAddress::UnixAddress() {
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
    }

    UnixAddress::UnixAddress(const std::string& path) {
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = path.size() + 1;
        if (!path.empty() && path[0] == '\0') {
            --m_length;
        }
        if (m_length > sizeof(m_addr.sun_path)) {
            throw std::logic_error("path too long");
        }
        memcpy(m_addr.sun_path, path.c_str(), m_length);
        m_length += offsetof(sockaddr_un, sun_path);
    }

    const sockaddr* UnixAddress::getAddr() const {
        return (sockaddr*)&m_addr;
    }

    sockaddr* UnixAddress::getAddr()  {
        return (sockaddr*)&m_addr;
    }

    socklen_t UnixAddress::getAddrLen() const {
        return m_length;
    }

    void UnixAddress::setAddrLen(uint32_t v) {
        m_length = v;
    }

    std::ostream& UnixAddress::insert(std::ostream& os) const {
        if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') {
            return os << "\\0" << std::string(m_addr.sun_path + 1, 
                m_length - offsetof(sockaddr_un, sun_path) - 1);
        }

        return os << m_addr.sun_path;
    }

    UnknownAddress::UnknownAddress(int family) {
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sa_family = family;
    }

    const sockaddr* UnknownAddress::getAddr() const {
        return &m_addr;
    }

    sockaddr* UnknownAddress::getAddr()  {
        return &m_addr;
    }

    socklen_t UnknownAddress::getAddrLen() const {
        return sizeof(m_addr);
    }
    std::ostream& UnknownAddress::insert(std::ostream& os) const {
        os << "[UnknownAddress family = " << m_addr.sa_family << "]";
        return os;
    }
}