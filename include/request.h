#ifndef __REQUEST_H__
#define __REQUEST_H__
#include <map>
class HTTPRequest {
public:
    HTTPRequest() {};
    std::map<std::string, std::string> headers;
};
#endif