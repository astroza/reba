#ifndef __HTTP__
#define __HTTP__
#include <boost/asio/ip/address.hpp>

namespace lake
{
namespace http
{
class Server
    {
    public:
        Server(){};
        int start(boost::asio::ip::address listen_address, unsigned short listen_port);
    };
} // namespace http
} // namespace lake
#endif