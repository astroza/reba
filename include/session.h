#ifndef __SESSION_H__
#define __SESSION_H__

#include <boost/asio/io_context.hpp>
#include <proto.h>

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(Protocol *protocol, boost::asio::io_context &io_context, tcp::socket socket) : current_protocol(protocol), socket(std::move(socket)), strand(io_context.get_executor()) {}
    void launch();
    void upgrade(Protocol *new_protocol);
    Protocol *getCurrentProtocol();
private:
	tcp::socket socket;
	boost::asio::strand<boost::asio::io_context::executor_type> strand;
    std::unique_ptr<Protocol> current_protocol;
};
#endif