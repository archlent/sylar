#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/macro.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>


static auto g_logger = SYLAR_LOG_ROOT();

int sock = 0;
 

void test_fiber() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    SYLAR_ASSERT(sock >= 0);
    SYLAR_LOG_INFO(g_logger) << "test_fiber sock = " << sock;
    sylar::setnonblocking(sock);
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.69.79", &addr.sin_addr);
    if (!connect(sock, (sockaddr*)&addr, sizeof(addr))) {

    } else if (errno == EINPROGRESS) {
        SYLAR_LOG_INFO(g_logger) << "add event error = " << errno << " " << strerror(errno);
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::READ, [](){
            SYLAR_LOG_INFO(g_logger) << "read call back";
        }); 
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, [](){
            SYLAR_LOG_INFO(g_logger) << "write call back";
            //sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::READ, [](){ SYLAR_LOG_DEBUG(g_logger) << "read"; } );
            sylar::IOManager::GetThis()->cancelEvent(sock, sylar::IOManager::READ);
            close(sock);
        });
        //sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, [](){ SYLAR_LOG_DEBUG(g_logger) << "1"; } );
        //sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::READ, [](){ SYLAR_LOG_DEBUG(g_logger) << "read"; } );
        //sylar::IOManager::GetThis()->cancelEvent(sock, sylar::IOManager::READ);
        //sylar::IOManager::GetThis()->delEvent(sock, sylar::IOManager::WRITE);
        //sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, [](){ SYLAR_LOG_DEBUG(g_logger) << "2"; } );
        //sylar::IOManager::GetThis()->delEvent(sock, sylar::IOManager::WRITE);
        //sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, [](){ SYLAR_LOG_DEBUG(g_logger) << "3"; } );
    } else {
        SYLAR_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test1() {
    sylar::IOManager iom;
    //sylar::IOManager iom(2, false);
    iom.schedule(test_fiber);
}

void test_timer() {
    sylar::IOManager iom(2);
    ptr<sylar::Timer> timer = iom.addTimer(1000, [&timer]() {
        static int i = 0;
        SYLAR_LOG_INFO(g_logger) << "hello timer";
        if (++i == 5) {
            timer->cacel();
        }
    }, true);
}

int main() {
    //test1();
    test_timer();
    return 0;
}