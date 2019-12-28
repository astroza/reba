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
	std::string src("console_log(console_log);");
	std::string src4("console_log(console_log);");
	char src2[] = "console_log(console_log);";
	V8Global v8_global;
	a(src);
	WorkerGroup *gw = new WorkerGroup(v8_global, std::string(src2));
	std::string src3("console_log(console_log);");
	//a(src3);
	sleep(5);
	std::cout << "heap4: " << src4 << std::endl;
	std::cout << "heap: " << src << std::endl;
	std::cout << "stack: " << src2 << std::endl;
	std::cout << "heap 2: " << src3 << std::endl;
	std::cout << "heap 2: " << src3 << std::endl;
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
