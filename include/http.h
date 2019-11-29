#ifndef __HTTP__H__
#define __HTTP__H__

#include <llhttp.h>
#include <map>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include <proto.h>
#include <request.h>

using boost::asio::ip::tcp;

#define MAX_FIELD_NAME_SIZE 64

class HTTP : public Protocol {
public:
    HTTP();
    std::unique_ptr<HTTPRequest> get_request(tcp::socket &socket, boost::asio::yield_context &yield);
    bool is_delegated_to_user();
private:
    llhttp_t http11_parser;
	llhttp_settings_t http11_settings;
    std::string _header_field_name;
    std::map<std::string, std::string> *request_headers;
    bool request_done;
	int on_http11_message_complete_cb(llhttp_t *parser);
	int on_http11_header_field_cb(llhttp_t *parser, const char *at, size_t length);
	int on_http11_header_value_cb(llhttp_t *parser, const char *at, size_t length);
    void init_http11_parser();
    bool consumeBuffer(const char *data, size_t size);
};
#endif