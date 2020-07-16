
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <http.h>
#include <router.h>
#include <worker.h>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

// Report a failure
void fail(beast::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}


reba::http::Server::Server(reba::Router *router) : router_(router) {
    threads_ = 4;
    io_context_ = std::make_unique<boost::asio::io_context>(threads_);
}

awaitable<void> reba::http::Server::DoSession(
    beast::tcp_stream stream)
{
    auto executor = co_await this_coro::executor;
    bool close = false;
    reba::http::Session session = { .stream_ = std::move(stream), .executor_ = executor};
    for (;;)
    {
        beast::http::request_parser<beast::http::empty_body> req_stage_0_;
        stream.expires_after(std::chrono::seconds(30));
        try {
            co_await beast::http::async_read(session.stream_, session.buffer_, req_stage_0_, use_awaitable);
        } catch(const std::exception& e) {
            // reba::log::print(reba::log::error, &session, e.what());
            break;
        }
        auto host = std::string((req_stage_0_.get())[beast::http::field::host]);
        auto worker_group_bind = router_->route_by_host(host);
        if(worker_group_bind == NULL) {
            break;
        }
        auto worker_group = static_cast<reba::WorkerGroup *>(worker_group_bind->getNativeObject());
        auto worker = worker_group->selectOrCreateWorker();
        boost::asio::post(worker->io_context_, [worker, &session]() {
            // This lambda function runs on worker thread
            worker->continueRequestProcessing(session);
        });
        // Wait for request processing
        co_await session.waitCue();
    }

    // Send a TCP shutdown
    stream.socket().shutdown(tcp::socket::shutdown_send);

    // At this point the connection is closed gracefully
}

awaitable<void> reba::http::Server::Listen(
    tcp::endpoint endpoint)
{
    auto executor = co_await this_coro::executor;
    // Open the acceptor
    tcp::acceptor acceptor(*io_context_);
    acceptor.open(endpoint.protocol());

    // Allow address reuse
    acceptor.set_option(net::socket_base::reuse_address(true));

    // Bind to the server address
    acceptor.bind(endpoint);

    // Start listening for connections
    acceptor.listen(net::socket_base::max_listen_connections);

    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(
            executor, [this, s = std::move(socket)]() mutable {
                return this->DoSession(beast::tcp_stream(std::move(s)));
            }, detached);
    }
}

int reba::http::Server::Start(boost::asio::ip::address listen_address, unsigned short listen_port)
{
    co_spawn(*io_context_, [this, listen_address, listen_port]() {
        return this->Listen(tcp::endpoint{listen_address, listen_port});
    }, detached);

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