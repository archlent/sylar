#pragma once 

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace sylar {
namespace http {

class HttpRequestParser {
public:
    HttpRequestParser();
    
    size_t execute(char* data, size_t len);
    int isFinished();
    int hasError();
    void setError(int v) { m_error = v; }
    ptr<HttpRequest> getData() const { return m_data; }
    uint64_t getContentLength();
    const http_parser& getParser() const { return m_parser; }
public:
    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpRequestMaxBodySize();
private:
    http_parser m_parser;
    ptr<HttpRequest> m_data;
    int m_error;                            /*1000 : invalid method
                                              1001 : invalid version
                                              1002 : invalid field   */
};

class HttpResponseParser {
public:
    HttpResponseParser();
    size_t execute(char* data, size_t len, bool chunck);
    int isFinished();
    int hasError();
    void setError(int v) { m_error = v; }
    auto getData() const { return m_data; }
    uint64_t getContentLength();

    const httpclient_parser& getParser() const { return m_parser; }

    static uint64_t GetHttpResponseBufferSize();
    static uint64_t GetHttpResponseMaxBodySize();
private:
    httpclient_parser m_parser;
    ptr<HttpResponse> m_data;
    int m_error;
};

} // namespace http
} // namespace sylar
