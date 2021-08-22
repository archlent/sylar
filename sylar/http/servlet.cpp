#include "servlet.h"
#include <fnmatch.h>

namespace sylar {
namespace http {

FunctionServlet::FunctionServlet(callback cb) : Servlet("FunctionServlet"), m_cb(cb) {
}

int32_t FunctionServlet::handle(ptr<HttpRequest> request, ptr<HttpResponse> response, ptr<HttpSession> session) {
    return m_cb(request, response, session);
}

ServletDispatch::ServletDispatch() : Servlet("ServletDispatch"),
    m_default(new NotFoundServlet) {
}

int32_t ServletDispatch::handle(ptr<HttpRequest> request, ptr<HttpResponse> response, ptr<HttpSession> session) {
    auto slt = getMatchedServerlet(request->getPath());
    if (slt) {
        slt->handle(request, response, session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string& uri, ptr<Servlet> slt) {
    WriteLock lock(m_mutex);
    m_datas[uri] = slt;
}

void ServletDispatch::addServlet(const std::string& uri, FunctionServlet::callback cb) {
    WriteLock lock(m_mutex);
    m_datas[uri].reset(new FunctionServlet(cb));
}

void ServletDispatch::addGlobServlet(const std::string& uri, ptr<Servlet> slt) {
    WriteLock lcok(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
        if (it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.emplace_back(uri, slt);
}

void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::callback cb) {
    return addGlobServlet(uri, ptr<Servlet>(new FunctionServlet(cb)));
}

void ServletDispatch::delServlet(const std::string& uri) {
    WriteLock lock(m_mutex);
    m_datas.erase(uri);
}

void ServletDispatch::delGlobServlet(const std::string& uri) {
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
        if (!fnmatch(it->first.c_str(), uri.c_str(), 0)) {
            m_globs.erase(it);
            break;
        }
    }
}


ptr<Servlet> ServletDispatch::getServlet(const std::string& uri) {
    ReadLock lock(m_mutex);
    auto it = m_datas.find(uri);
    return it == m_datas.end() ? nullptr : it->second;
}

ptr<Servlet> ServletDispatch::getGlobServlet(const std::string& uri) {
    ReadLock lock(m_mutex);
    for (const auto& [x, y] : m_globs) {
        if (!fnmatch(x.c_str(), uri.c_str(), 0)) {
            return y;
        }
    }
    return nullptr;
}

ptr<Servlet> ServletDispatch::getMatchedServerlet(const std::string& uri) {
    ReadLock lock(m_mutex);
    auto it = m_datas.find(uri);
    if (it != m_datas.end()) {
        return it->second;
    }
    for (const auto& [x, y] : m_globs) {
        if (!fnmatch(x.c_str(), uri.c_str(), 0)) {
            return y;
        }
    }
    return m_default;
}


NotFoundServlet::NotFoundServlet() : Servlet("NotFoundServlet") {

}

int32_t NotFoundServlet::handle(ptr<HttpRequest> request, ptr<HttpResponse> response, ptr<HttpSession> session) {
    static const std::string BODY1 = "<html><head>\n"
                                     "<title>404 Not Found</title>\n"
                                     "</head><body>\n"
                                     "<h1>Not Found</h1>\n"
                                     "<p>The requested URL ";
    static const std::string BODY2 = " was not found on this server.</p>\n"
                                     "</body></html>";
    static const std::string RSP_BODY = BODY1 + request->getPath() + BODY2;
    response->setStatus(HttpStatus::NOT_FOUND);
    response->setHeader("Server", "sylar/1.0.0");
    response->setHeader("Content-Type", "text/html");
    response->setBody(RSP_BODY);

    return 0;
}

}
}

