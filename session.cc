#include <session.h>
#include <iostream>

void Session::upgrade(Protocol *new_protocol)
{
    current_protocol = std::unique_ptr<Protocol>(new_protocol);
}

Protocol *Session::getCurrentProtocol()
{
    return current_protocol.get();
}

void Session::launch()
{
    auto self(shared_from_this());
    boost::asio::spawn(strand,
                       [this, self](boost::asio::yield_context yield) {
                           auto request = current_protocol->get_request(socket, yield);
                           std::cout << "REQUEST" << std::endl;
                           socket.close();
                       });
}