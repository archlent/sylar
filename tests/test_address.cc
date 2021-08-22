#include "sylar/address.h"
#include "sylar/macro.h"

static auto g_logger = SYLAR_LOG_ROOT();

void testIPv6() {
    auto addr = sylar::IPv6Address::Create("2001:0db8:85a3::8a2e:0370:7334");
    SYLAR_LOG_INFO(g_logger) << "IPAddress        = " << addr->toString();
    SYLAR_LOG_INFO(g_logger) << "broadcastAddress = " << addr->broadcastAddress(44)->toString();
    SYLAR_LOG_INFO(g_logger) << "networkAddress   = " << addr->networdAddress(44)->toString();
    SYLAR_LOG_INFO(g_logger) << "subnetMask       = " << addr->subnetMask(44)->toString();
}

void testUnixAddress() {
    sylar::UnixAddress ua;
    SYLAR_LOG_INFO(g_logger) << ua.getAddrLen();
}

void testIpv4() {
    auto addr = sylar::IPv4Address::Create("172.17.237.213");
    if (addr) {
        SYLAR_LOG_INFO(g_logger) << "IPAddress        = " << addr->toString();
        SYLAR_LOG_INFO(g_logger) << "broadcastAddress = " << addr->broadcastAddress(20)->toString();
        SYLAR_LOG_INFO(g_logger) << "networkAddress   = " << addr->networdAddress(20)->toString();
        SYLAR_LOG_INFO(g_logger) << "subnetMask       = " << addr->subnetMask(20)->toString();
    }
    //auto baidu = sylar::IPAddress::Create("www.baidu.com");
    auto baidu = sylar::IPAddress::Create("103.235.46.39", 80);
    if (baidu) { 
        SYLAR_LOG_INFO(g_logger) << baidu->toString();
    }
}

void test() {
    std::vector<ptr<sylar::Address>> addrs;
    bool flag = sylar::Address::Lookup(addrs, "www.baidu.com:ftp");
    SYLAR_ASSERT(flag);
    flag = sylar::Address::Lookup(addrs, "baidu.com");
    SYLAR_ASSERT(flag);
    for (auto& addr : addrs) {
        SYLAR_LOG_INFO(g_logger) << addr->toString();
    }

    auto addpsptr = sylar::Address::Lookup("www.sylar.top:80");
    SYLAR_ASSERT(addpsptr);
    for (auto& addr : *addpsptr) {
        SYLAR_LOG_INFO(g_logger) << "www.sylar.top ==> " << addr->toString();
    }
}

void test_iface() {
    auto opt = sylar::Address::GetInterfaceAddress();
    SYLAR_ASSERT(opt);
    bool flag = false;
    std::string_view sv;
    for (auto& [x, y] : *opt) {
        if (!flag) {
            sv = x;
            flag = true;
        }
        SYLAR_LOG_INFO(g_logger) << x << " - " << y.first->toString() << " - " << y.second;
    }
    auto wlan0 = sylar::Address::GetInterfaceAddress(sv);
    if (!wlan0) {
        SYLAR_LOG_INFO(g_logger) << "No such name as " << sv;
    }
    for (auto& [x, y] : *wlan0) {
        SYLAR_LOG_INFO(g_logger) << x->toString() << " - " << y;
    }
}

int main() {
    testIPv6();
    //testUnixAddress();
    SYLAR_LOG_DEBUG(g_logger) << "--------------------------------------------------------";
    testIpv4();
    SYLAR_LOG_DEBUG(g_logger) << "---------------------------------------------------------";
    test();
    SYLAR_LOG_DEBUG(g_logger) << "---------------------------------------------------------";
    test_iface();
}
