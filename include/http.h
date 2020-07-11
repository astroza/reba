#ifndef __HTTP__
#define __HTTP__
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <boost/asio/ip/address.hpp>
#include <router.h>

#include <boost/asio/this_coro.hpp>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
using boost::asio::awaitable;

namespace reba
{
namespace http
{
struct Session
{
    beast::http::request_parser<beast::http::empty_body> req_stage_0_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    net::detail::awaitable_handler<net::executor> *handler_;
    const net::executor &executor_;
    ::http::response<::http::string_body> response_;

    auto waitCue() {
        return [&handler = this->handler_](auto *frame) {
            handler = new net::detail::awaitable_handler<net::executor>(frame->detach_thread());
            return static_cast<net::detail::awaitable_thread<net::executor> *>(nullptr);
        };
    }

    void setCue() {
        boost::asio::post(executor_, [handler = this->handler_]() {
            (*handler)();
            free(handler);
        });
    }
   // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda
    {
        template<bool isRequest, class Body, class Fields>
        void
        operator()(struct Session& session, ::http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<
                ::http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            session.res_ = sp;

            // Write the response
            ::http::async_write(
                session.stream_,
                *sp, beast::bind_front_handler(
                [&session](boost::system::error_code& err, std::size_t sz) {
                    session.setCue();
                }));
        }
    };

    void send(::http::response<::http::string_body> res) {
        lambda_(*this, std::move(res));
    }
    send_lambda lambda_;
    std::shared_ptr<void> res_;
};

class Server
    {
    public:
        Server(reba::Router *router);
        int Start(net::ip::address listen_address, unsigned short listen_port);
        awaitable<void> Listen(tcp::endpoint endpoint);
        awaitable<void> DoSession(beast::tcp_stream stream);
    private:
        int threads_;
        reba::Router *router_;
        std::unique_ptr<boost::asio::io_context> io_context_;
    };
} // namespace http
} // namespace reba
#endif