#pragma once

#include "sylar/log.h"
#include "http.h"
#include "http_session.h"
#include "sylar/mutex.h"

namespace sylar {
namespace http {

class Servlet {
public:
    Servlet(const std::string& name) : m_name(name) {}
    virtual ~Servlet() {}
    virtual int32_t handle(ptr<HttpRequest> request, ptr<HttpResponse> response, ptr<HttpSession> session) = 0;
    const std::string& getName() const { return m_name; }
protected:
    std::string m_name;
};

class FunctionServlet : public Servlet {
public:
    using callback = std::function<int32_t(ptr<HttpRequest> request,
        ptr<HttpResponse> response, ptr<HttpSession> session)>;
    FunctionServlet(callback cb);
    int32_t handle(ptr<HttpRequest> request, ptr<HttpResponse> response, ptr<HttpSession> session) override;
private:
    callback m_cb;
};


class ServletDispatch : public Servlet {
public:
    using RWMutexType = shared_mutex; 

    ServletDispatch();
    int32_t handle(ptr<HttpRequest> request, ptr<HttpResponse> response, ptr<HttpSession> session) override;
    void addServlet(const std::string& uri, ptr<Servlet> slt);
    void addServlet(const std::string& uri, FunctionServlet::callback cb);
    void addGlobServlet(const std::string& uri, ptr<Servlet> slt);
    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    ptr<Servlet> getDefault() const { return m_default; }
    void setDefault(ptr<Servlet> v) { m_default = v; }

    ptr<Servlet> getServlet(const std::string& uri);
    ptr<Servlet> getGlobServlet(const std::string& uri);

    ptr<Servlet> getMatchedServerlet(const std::string& uri);
private:
    RWMutexType m_mutex;
    // uri(/sylar/xxx) -> servlet 精确
    std::unordered_map<std::string, ptr<Servlet>> m_datas;
    // uri(/sylar/*) -> servlet
    std::vector<std::pair<std::string, ptr<Servlet>>> m_globs;
    ptr<Servlet> m_default;
};

class NotFoundServlet : public Servlet {
public:
    NotFoundServlet();
    int32_t handle(ptr<HttpRequest> request, ptr<HttpResponse> response, ptr<HttpSession> session) override;
};

} // namespace http
} // namespace sylar
