#ifndef __PROTO__H__
#define __PROTO__H__
#include <cstddef>
#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <request.h>

using boost::asio::ip::tcp;

class Protocol {
public:
    virtual std::unique_ptr<HTTPRequest> get_request(tcp::socket &socket, boost::asio::yield_context &yield) = 0;
    virtual bool is_delegated_to_user() = 0;
};
#endif