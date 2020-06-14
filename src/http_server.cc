
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#if BOOST_VERSION >= 107200
#include <boost/range/begin.hpp> // workaround for boost 1.72 bug
#include <boost/range/end.hpp>   // workaround for boost 1.72 bug
#endif
#include <boost/asio/spawn.hpp>
#include <boost/config.hpp>
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
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <
    class Body, class Allocator,
    class Send>
void handle_request(
    http::request<Body, http::basic_fields<Allocator>> &&req,
    Send &&send)
{

    // Returns a server error response
    auto const server_error =
        [&req](beast::string_view what) {
            http::response<http::string_body> res{http::status::internal_server_error, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
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

// Handles an HTTP server connection
void do_session(
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
        http::request<http::string_body> req;
        http::async_read(stream, buffer, req, yield[ec]);
        if (ec == http::error::end_of_stream)
            break;
        if (ec)
            return fail(ec, "read");
        auto host = std::string(req[http::field::host]);
        auto worker_group_bind = lake::g_router.route_by_host(host);
        if(worker_group_bind) {
            auto worker_group = static_cast<lake::WorkerGroup *>(worker_group_bind->GetNativeObject());
            worker_group->delegate_request();
        }
        // Send the response
        handle_request(std::move(req), [&stream, &close, &ec, yield]<bool isRequest, class Body, class Fields>(http::message<isRequest, Body, Fields> &&msg) {
            close = msg.need_eof();
            http::serializer<isRequest, Body, Fields> sr{msg};
            http::async_write(stream, sr, yield[ec]);
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

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
void do_listen(
    net::io_context &ioc,
    tcp::endpoint endpoint,
    net::yield_context yield)
{
    beast::error_code ec;

    // Open the acceptor
    tcp::acceptor acceptor(ioc);
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
        tcp::socket socket(ioc);
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
                    &do_session,
                    beast::tcp_stream(std::move(socket)),
                    std::placeholders::_1));
        }
    }
}

namespace lake
{
namespace http
{
    int Server::start(boost::asio::ip::address listen_address, unsigned short listen_port)
    {
        auto const threads = 4;
        // The io_context is required for all I/O
        net::io_context ioc{threads};

        // Spawn a listening port
        net::spawn(ioc,
                    std::bind(
                        &do_listen,
                        std::ref(ioc),
                        tcp::endpoint{listen_address, listen_port},
                        std::placeholders::_1));

        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        v.reserve(threads - 1);
        for (auto i = threads - 1; i > 0; --i)
            v.emplace_back(
                [&ioc] {
                    ioc.run();
                });
        ioc.run();

        return EXIT_SUCCESS;
    }
} // namespace http
} // namespace lake