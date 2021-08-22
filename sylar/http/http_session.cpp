#include "http_session.h"
#include "http_parser.h"

namespace sylar {
namespace http {
    
HttpSession::HttpSession(ptr<Socket> sock, bool owner)
    : SockStream(sock, owner) {
}

ptr<HttpRequest> HttpSession::recvRequest() {
    ptr<HttpRequestParser> parser(new HttpRequestParser);
    uint64_t buffer_size = HttpRequestParser::GetHttpRequestBufferSize();
    //uint64_t buffer_size = 100;
    ptr<char> buffer(new char[buffer_size], [](char* ptr){
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
        size_t nparse = parser->execute(data, len);         // execute会改变data的位置
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
    int64_t length = parser->getContentLength();        // body长度
    if (length > 0) {
        std::string body;
        body.resize(length);
//            int len = 0;
//            if (length >= offset) {
//                memcpy(&body[0], data, offset);
//                len = offset;
//            } else {
//                memcpy(&body[0], data, length);
//                len = length;
//            }
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
    parser->getData()->init();                      //  初始化连接状态
    return parser->getData();
}

int HttpSession::sendResponse(ptr<HttpResponse> rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}

}
}
