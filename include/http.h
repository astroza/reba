#ifndef __HTTP__
#define __HTTP__
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
#include <boost/asio/ip/address.hpp>
#include <router.h>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace reba
{
namespace http
{
class Server
    {
    public:
        Server(reba::Router *router);
        int Start(net::ip::address listen_address, unsigned short listen_port);
        void Listen(tcp::endpoint endpoint, net::yield_context yield);
        void DoSession(beast::tcp_stream &stream, net::yield_context yield);
    private:
        int threads_;
        reba::Router *router_;
        std::unique_ptr<boost::asio::io_context> io_context_;
    };
} // namespace http
} // namespace reba
#endif