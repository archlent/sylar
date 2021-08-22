#include "sylar/http/http_server.h"

static auto g_logger = SYLAR_LOG_ROOT();

void run() {
    ptr<sylar::http::HttpServer> server(new sylar::http::HttpServer);
    auto addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr)) {
	    sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx", [](ptr<sylar::http::HttpRequest> req,
            ptr<sylar::http::HttpResponse> rsp, ptr<sylar::http::HttpSession>) {
        rsp->setBody(req->toString());
        return 0;
    });
    sd->addGlobServlet("/sylar/*", [](ptr<sylar::http::HttpRequest> req,
            ptr<sylar::http::HttpResponse> rsp, ptr<sylar::http::HttpSession>) {
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });
    server->start();
}

int main() {
    sylar::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
