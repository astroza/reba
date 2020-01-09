#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <memory>
#include <http.h>
#include <session.h>

#include <worker_group.h>
#include <unistd.h>
using boost::asio::ip::tcp;

void a(std::string s) {
	std::cout << s.size();
}
int main(int argc, char *argv[])
{
	lake::v8_init();
	WorkerGroup *gov_worker_group = new WorkerGroup("for(var i = 0; i < 2; i++) new WorkerGroup('log(\"test\");'); var t = new Timer();", true);
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: echo_server <port>\n";
			return 1;
		}

		boost::asio::io_context io_context;
		boost::asio::spawn(io_context,
						   [&](boost::asio::yield_context yield) {
							   tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
							   do
							   {
								   boost::system::error_code ec;
								   tcp::socket socket(io_context);
								   acceptor.async_accept(socket, yield[ec]);
								   if (!ec)
								   {
									   std::make_shared<Session>(new HTTP(), io_context, std::move(socket))->launch();
								   }
							   } while (true);
						   });

		io_context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
