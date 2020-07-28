
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

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

reba::http::Server::Server(reba::Router* router)
    : router_(router)
{
    threads_ = std::thread::hardware_concurrency();
}

awaitable<void> reba::http::Server::DoSession(
    beast::tcp_stream stream)
{
    auto executor = co_await this_coro::executor;
    reba::http::Session session = { .stream_ = std::move(stream), .cue_ = boost::asio::steady_timer(executor, std::chrono::steady_clock::now() + std::chrono::hours(24 * 7)) };
    for (;;) {
        beast::http::request_parser<beast::http::empty_body> req_stage_0_;
        stream.expires_after(std::chrono::seconds(30));
        try {
            co_await beast::http::async_read(session.stream_, session.buffer_, req_stage_0_, use_awaitable);
        } catch (const std::exception& e) {
            // reba::log::print(reba::log::error, &session, e.what());
            break;
        }
        session.request_ = req_stage_0_.release();
        auto host = std::string(session.request_[beast::http::field::host]);
        auto worker_group_bind = router_->route_by_host(host);
        if (worker_group_bind == NULL) {
            break;
        }
        auto worker_group = static_cast<reba::WorkerGroup*>(worker_group_bind->getNativeObject());
        auto worker = worker_group->selectOrCreateWorker();
        boost::asio::post(worker->io_context_, [worker, &session]() {
            // This lambda function runs on worker thread
            worker->continueRequestProcessing(session);
        });
        // Wait for request processing
        try {
            co_await session.cue_.async_wait(use_awaitable);
        } catch (const boost::system::system_error& se) {
            if(se.code() != boost::asio::error::operation_aborted) {
                // reba::log::print(reba::log::error, &session, se.what());
            }
        }
        if(!session.request_.keep_alive()) {
            break;
        }
    }
    // Send a TCP shutdown
    session.stream_.socket().shutdown(tcp::socket::shutdown_send);
}

awaitable<void> reba::http::Server::Listen(
    tcp::acceptor &root_acceptor)
{
    auto executor = co_await this_coro::executor;
    auto acceptor = tcp::acceptor(executor, root_acceptor.local_endpoint().protocol(), root_acceptor.native_handle());
    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(
            executor, [this, s = std::move(socket)]() mutable {
                return this->DoSession(beast::tcp_stream(std::move(s)));
            },
            detached);
    }
}

int reba::http::Server::Start(boost::asio::ip::address listen_address, unsigned short listen_port)
{
    boost::asio::io_context dummy_io_context;
    tcp::acceptor acceptor(dummy_io_context);
    tcp::endpoint endpoint { listen_address, listen_port };
    std::vector<std::thread> threads_vector;

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(net::socket_base::max_listen_connections);
    threads_vector.reserve(threads_ - 1);

    auto thread_main = [this, &acceptor] {
        boost::asio::io_context io_context;
        co_spawn(
            io_context, [this, &acceptor]() {
                return this->Listen(acceptor);
            },
            detached);
        io_context.run();
    };

    /**
     * io_context shared to many threads has a issue when using coroutines and timer.cancel(),
     * so now I share just the underlying listen socket
     */
    for (int i = 0; i < threads_ - 1; i++) {
        threads_vector.emplace_back(thread_main);
    }
    thread_main();
    return EXIT_SUCCESS;
}