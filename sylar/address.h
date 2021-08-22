#pragma once

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h> 
#include <iostream>
#include <arpa/inet.h>
#include <sys/un.h>
#include <optional>
#include "log.h"

namespace sylar {
    class IPAddress;
    class Address {
    public:
        static ptr<Address> Create(const sockaddr* addr, socklen_t addrlen);
        static bool Lookup(std::vector<ptr<Address>>& result, const std::string& host, int family = AF_INET,
                            int type = 0, int protocol = 0);
        static std::optional<std::vector<ptr<Address>>> Lookup(std::string_view host, int family = AF_INET,
                            int type = 0, int protocol = 0);
        static ptr<Address> LookupAny(std::string_view host, int family = AF_INET, int type = 0, int protocol = 0);
        static ptr<IPAddress> LookupAnyIPAddress(std::string_view host, int family = AF_INET,
                                                    int type = 0, int protocol = 0);
        static bool GetInterfaceAddress(std::multimap<std::string_view, std::pair<ptr<Address>, uint32_t>>& result, 
                                        int family = AF_INET);
        static std::optional<std::multimap<std::string_view, std::pair<ptr<Address>, uint32_t>>>
        GetInterfaceAddress(int family = AF_INET);
        static bool GetInterfaceAddress(std::vector<std::pair<ptr<Address>, uint32_t>>& result, 
                std::string_view iface, int family = AF_INET);
        static std::optional<std::vector<std::pair<ptr<Address>, uint32_t>>>
        GetInterfaceAddress(std::string_view iface, int family = AF_INET);
        virtual ~Address() {}
        int getFamily() const;
        virtual const sockaddr* getAddr() const = 0;
        virtual sockaddr* getAddr() = 0;
        virtual socklen_t getAddrLen() const = 0;

        virtual std::ostream& insert(std::ostream& os) const {
            return os;
        }
        std::string toString() const;
        
        bool operator<(const Address& rhs) const;
        bool operator==(const Address& rhs) const;
        bool operator!=(const Address& rhs) const;
    };

    class IPAddress : public Address {
    public:
        static ptr<IPAddress> Create(const char* address, uint16_t port = 0);
        virtual ptr<IPAddress> broadcastAddress(uint32_t prefix_len) = 0;
        virtual ptr<IPAddress> networdAddress(uint32_t prefix_len) = 0;
        virtual ptr<IPAddress> subnetMask(uint32_t prefix_len) = 0;

        virtual uint32_t getPort() const = 0;
        virtual void setPort(uint16_t v) = 0;
    };

    class IPv4Address : public IPAddress {
    public:
        static ptr<IPv4Address> Create(const char* address, uint16_t port = 0);
        IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
        IPv4Address(const sockaddr_in& rhs) : m_addr(rhs) {}
        const sockaddr* getAddr() const override;
        sockaddr* getAddr() override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;

        ptr<IPAddress> broadcastAddress(uint32_t prefix_len) override;
        ptr<IPAddress> networdAddress(uint32_t prefix_len) override;
        ptr<IPAddress> subnetMask(uint32_t prefix_len) override;

        uint32_t getPort() const override;
        void setPort(uint16_t v) override;
    
    private:
        sockaddr_in m_addr;
    };

    class IPv6Address : public IPAddress {
    public:
        static ptr<IPv6Address> Create(const char* address, uint16_t port = 0);
        IPv6Address();
        IPv6Address(const sockaddr_in6& rhs) : m_addr(rhs) {}
        IPv6Address(const uint8_t address[16], uint16_t port = 0);
        const sockaddr* getAddr() const override;
        sockaddr* getAddr() override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;

        ptr<IPAddress> broadcastAddress(uint32_t prefix_len) override;
        ptr<IPAddress> networdAddress(uint32_t prefix_len) override;
        ptr<IPAddress> subnetMask(uint32_t prefix_len) override;

        uint32_t getPort() const override;
        void setPort(uint16_t v) override;
    
    private:
        sockaddr_in6 m_addr;
    };

    class UnixAddress : public Address {
    public:
        UnixAddress();
        UnixAddress(const std::string& path);
        const sockaddr* getAddr() const override;
        sockaddr* getAddr() override;
        socklen_t getAddrLen() const override;
        void setAddrLen(uint32_t v);
        std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr_un m_addr;
        socklen_t m_length; 
    };

    class UnknownAddress : public Address {
    public:
        UnknownAddress(const sockaddr& addr) : m_addr(addr) { }
        UnknownAddress(int family);
        const sockaddr* getAddr() const override;
        sockaddr* getAddr() override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr m_addr;
    };

    inline std::ostream& operator<<(std::ostream& os, const Address& addr) {
        return addr.insert(os);
    }
}