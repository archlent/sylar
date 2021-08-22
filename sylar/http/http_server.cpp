#include "http_server.h"
#include "http_session.h"

namespace sylar {
namespace http {

static auto g_logger = SYLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, IOManager* worker, IOManager* accept_worker)
    : TcpServer(worker, accept_worker), m_isKeepalive(keepalive),
      m_dispatch(new ServletDispatch) {
}

void HttpServer::handleClient(ptr<Socket> client) {
    ptr<HttpSession> seesion(new HttpSession(client));
    do {
        auto req = seesion->recvRequest();
        if (!req) {
            SYLAR_LOG_WARN(g_logger) << "recv http request fail, error = "
             << errno << " errstr = " << strerror(errno) << " client = "
             << *client;
            break;
        }
        ptr<HttpResponse> rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));

        m_dispatch->handle(req, rsp, seesion);
//        rsp->setBody("hello sylar");
//
//        SYLAR_LOG_INFO(g_logger) << "request: " << std::endl << *req;
//        SYLAR_LOG_INFO(g_logger) << "response: " << std::endl << *rsp;

        seesion->sendResponse(rsp);
    } while (m_isKeepalive);
    seesion->close();
}   

} // namespace http
} // namespace sylar
