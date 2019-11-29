#include <session.h>
#include <iostream>

void Session::upgrade(Protocol *newProtocol)
{
    currentProtocol = std::unique_ptr<Protocol>(newProtocol);
}

Protocol *Session::getCurrentProtocol()
{
    return currentProtocol.get();
}

void Session::launch()
{
    auto self(shared_from_this());
    boost::asio::spawn(strand,
                       [this, self](boost::asio::yield_context yield) {
                           auto request = currentProtocol->get_request(socket, yield);
                           std::cout << "REQUEST" << std::endl;
                           socket.close();
                       });
}