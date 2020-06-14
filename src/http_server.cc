
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <http.h>
#include <router.h>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

template <
    class Body, class Allocator,
    class Send>
void handle_request(
    beast::http::request<Body, beast::http::basic_fields<Allocator>> &&req,
    Send &&send)
{

    // Returns a server error response
    auto const server_error =
        [&req](beast::string_view what) {
            beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, req.version()};
            res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "An error occurred: '" + std::string(what) + "'";
            res.prepare_payload();
            return res;
        };
    return send(server_error("Not implemented"));
}

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}


lake::http::Server::Server(lake::Router *router) : router_(router) {
    threads_ = 4;
    io_context_ = std::make_unique<boost::asio::io_context>(threads_);
}

void lake::http::Server::Listen(
    tcp::endpoint endpoint,
    net::yield_context yield)
{
    beast::error_code ec;

    // Open the acceptor
    tcp::acceptor acceptor(*io_context_);
    acceptor.open(endpoint.protocol(), ec);
    if (ec)
        return fail(ec, "open");

    // Allow address reuse
    acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
        return fail(ec, "set_option");

    // Bind to the server address
    acceptor.bind(endpoint, ec);
    if (ec)
        return fail(ec, "bind");

    // Start listening for connections
    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
        return fail(ec, "listen");

    for (;;)
    {
        tcp::socket socket(*io_context_);
        acceptor.async_accept(socket, yield[ec]);
        if (ec)
        {
            fail(ec, "accept");
        }
        else
        {
            net::spawn(
                acceptor.get_executor(),
                std::bind(
                    &Server::DoSession,
                    this,
                    beast::tcp_stream(std::move(socket)),
                    std::placeholders::_1));
        }
    }
}

void lake::http::Server::DoSession(
    beast::tcp_stream &stream,
    net::yield_context yield)
{
    bool close = false;
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    for (;;)
    {
        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request
        beast::http::request<beast::http::string_body> req;
        beast::http::async_read(stream, buffer, req, yield[ec]);
        if (ec == beast::http::error::end_of_stream)
            break;
        if (ec)
            return fail(ec, "read");
        auto host = std::string(req[beast::http::field::host]);
        auto worker_group_bind = router_->route_by_host(host);
        if(worker_group_bind) {
            auto worker_group = static_cast<lake::WorkerGroup *>(worker_group_bind->GetNativeObject());
            worker_group->delegate_request();
        }
        // Send the response
        handle_request(std::move(req), [&stream, &close, &ec, yield]<bool isRequest, class Body, class Fields>(beast::http::message<isRequest, Body, Fields> &&msg) {
            close = msg.need_eof();
            beast::http::serializer<isRequest, Body, Fields> sr{msg};
            beast::http::async_write(stream, sr, yield[ec]);
        });
        if (ec)
            return fail(ec, "write");
        if (close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }

    // Send a TCP shutdown
    stream.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

int lake::http::Server::Start(boost::asio::ip::address listen_address, unsigned short listen_port)
{
    net::spawn(*io_context_,
                std::bind(
                    &Server::Listen,
                    this,
                    tcp::endpoint{listen_address, listen_port},
                    std::placeholders::_1));

    std::vector<std::thread> threads_vector;
    threads_vector.reserve(threads_ - 1);
    for (int i = threads_ - 1; i > 0; --i)
        threads_vector.emplace_back(
            [this] {
                this->io_context_->run();
            });
    io_context_->run();

    return EXIT_SUCCESS;
}