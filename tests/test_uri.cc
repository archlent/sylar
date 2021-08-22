#include "sylar/uri.h"
#include <iostream>

int main() {
    ptr<sylar::Uri> uri = sylar::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;


    return 0;
}