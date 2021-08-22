#include "sylar/http/http.h"

static auto g_logger = SYLAR_LOG_ROOT();

void test_requset() {
    ptr<sylar::http::HttpRequest> req(new sylar::http::HttpRequest()); 
    req->setHeader("host", "www.sylar.top");
    req->setBody("hello sylar");

    req->dump(std::cout) << std::endl;  
}

void test_response() {
    ptr<sylar::http::HttpResponse> rsp(new sylar::http::HttpResponse());
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello sylar");
    rsp->setClose(false);
    rsp->setStatus((sylar::http::HttpStatus)400);
    rsp->dump(std::cout) << std::endl;
}
int main() {
    test_requset();
    test_response();
}