#pragma once

#include "sylar/socket_stream.h"
#include "http.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

class HttpSession : public SockStream {
public:
    HttpSession(ptr<Socket> sock, bool owner = true);
    ptr<HttpRequest> recvRequest();
    int sendResponse(ptr<HttpResponse> rsp);            // 发送响应报文
};
}
}