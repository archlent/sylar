#include "http_connection.h"
#include "http_parser.h"

namespace sylar {
namespace http {

 auto g_logger = SYLAR_LOG_NAME("system");

HttpConnection::HttpConnection(ptr<Socket> sock, bool owner)
    : SockStream(sock, owner) {
}

ptr<HttpResponse> HttpConnection::recvResponse() {
    ptr<HttpResponseParser> parser(new HttpResponseParser);
    uint64_t buffer_size = HttpResponseParser::GetHttpResponseBufferSize();
    //uint64_t buffer_size = 100;
    ptr<char> buffer(new char[buffer_size + 1], [](char* ptr){
        delete[] ptr;
    });
    char* data = buffer.get();
    int offset = 0;
    while (true) {
        int len = read(data + offset, buffer_size - offset);  // data + offset： 实际读到的位置
        if (len <= 0) {
            close();
            return nullptr;
        }
        len += offset;                                      // 刚读完还没解析的长度
        data[len] = '\0';
        size_t nparse = parser->execute(data, len, false);         // execute会改变data的位置
        if (parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;                              // 刚解析完的还没有解析的长度
        if (offset == (int)buffer_size) {                   // 完全解析不了，可能是恶意请求
            close();
            return nullptr;
        }
        if (parser->isFinished()) {
            break;
        }
    }
    auto& client_parser = parser->getParser();
    std::string body;
    if(client_parser.chunked) {
        int len = offset;
        do {
            bool begin = true;
            do {
                if(!begin || len == 0) {
                    int rt = read(data + len, buffer_size - len);
                    if(rt <= 0) {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if(parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if(len == (int)buffer_size) {
                    close();
                    return nullptr;
                }
                begin = false;
            } while(!parser->isFinished());
            //len -= 2;
            
            SYLAR_LOG_DEBUG(g_logger) << "content_len=" << client_parser.content_len;
            if(client_parser.content_len + 2 <= len) {
                body.append(data, client_parser.content_len);
                memmove(data, data + client_parser.content_len + 2
                        , len - client_parser.content_len - 2);
                len -= client_parser.content_len + 2;
            } else {
                body.append(data, len);
                int left = client_parser.content_len - len + 2;
                while(left > 0) {
                    int rt = read(data, left > (int)buffer_size ? (int)buffer_size : left);
                    if(rt <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        } while(!client_parser.chunks_done);
        parser->getData()->setBody(body);
    } else {
        int64_t length = parser->getContentLength();        // body长度
        if (length > 0) {
            std::string body;
            body.resize(length);
            int len = std::min(length, (int64_t)offset);
            memcpy(body.data(), data, len);
            length -= offset;
            if (length > 0) {                               // 还没读完但是已经解析完头部
                if (readFixSize(&body[len], length) <= 0) {
                    close();
                    return nullptr;
                }
            }
            parser->getData()->setBody(body);
        }
    }
    return parser->getData();
}

int HttpConnection::sendRequest(ptr<HttpRequest> rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}


 ptr<HttpResult> HttpConnection::DoGet(const std::string& uri, uint64_t timeout_ms, 
        const std::map<std::string, std::string>& headers, const std::string& body) {
    return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

 ptr<HttpResult> HttpConnection::DoGet(ptr<Uri> uri, uint64_t timeout_ms,
        const std::map<std::string, std::string>& headers, const std::string& body) {
    return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

 ptr<HttpResult> HttpConnection::DoPost(const std::string& uri, uint64_t timeout_ms, 
        const std::map<std::string, std::string>& headers, const std::string& body) {
    return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}

 ptr<HttpResult> HttpConnection::DoPost(ptr<Uri> uri, uint64_t timeout_ms, 
        const std::map<std::string, std::string>& headers, const std::string& body) {
    return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}

 ptr<HttpResult> HttpConnection::DoRequest(HttpMethod method, const std::string& url, 
        uint64_t timeout_ms, const std::map<std::string, std::string>& headers, 
        const std::string& body) {
    auto uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URI, 
         nullptr, "invalid uri" + url);
    }
    return DoRequest(method, uri, timeout_ms, headers, body);
}

 ptr<HttpResult> HttpConnection::DoRequest(HttpMethod method, ptr<Uri> uri, 
        uint64_t timeout_ms, const std::map<std::string, std::string>& headers, 
        const std::string& body) {
    auto req = std::make_shared<HttpRequest>();
    req->setPath(uri->getPath());
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());
    req->setMethod(method);
    bool has_host = false;
    for (auto& [h, v] : headers) {
        if (strcasecmp(h.c_str(), "connection") == 0) {
            if (strcasecmp(v.c_str(), "keep_alive") == 0) {
                req->setClose(false);
            }
            continue;
        }
        if (!has_host && strcasecmp(h.c_str(), "host") == 0) {
            has_host = !v.empty();
        }
        req->setHeader(h, v);
    }
    if (!has_host) {
        req->setHeader("Host", uri->getHost());
    }
    req->setBody(body);
    return DoRequest(req, uri, timeout_ms);
}

 ptr<HttpResult> HttpConnection::DoRequest(ptr<HttpRequest> req, ptr<Uri> uri, 
        uint64_t timeout_ms) {
    auto addr = uri->createAddress();
    if (!addr) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST, 
         nullptr, "invalid host: " + uri->getHost());
    }
    ptr<Socket> sock = Socket::CreateTCP(addr);
    if (!sock) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL, 
         nullptr, "create socket fail: " + addr->toString()
         + " errno = " + std::to_string(errno) + " errstr = " + std::string(strerror(errno)));
    }
    if (!sock->connect(addr)) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL, 
         nullptr, "connect fail: " + addr->toString());
    }
    sock->setRecvTimeout(timeout_ms);
    auto conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);
    if (rt == 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE, 
         nullptr, "send request closed by peer: " + addr->toString());
    } else if (rt < 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR, 
         nullptr, "send requst socket error, errno = " + std::to_string(errno) 
         + " errstr = " + std::string(strerror(errno)));
    }
    auto rsp = conn->recvResponse();
    if (!rsp) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT, 
         nullptr, "recv response timeout: " + addr->toString() + ", timeout_ms = "
         + std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "OK");
}



}
}
