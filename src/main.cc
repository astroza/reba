
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast/http.hpp>
#include <http.h>
#include <worker_group.h>
#include <lakeapi.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
	{
		std::cerr << "Usage: lake <governator script file> <port>\n";
		return 1;
	}

	lake::engine::init();
	std::ifstream gov_script_file { argv[1] };
	std::string gov_script { std::istreambuf_iterator<char>(gov_script_file), std::istreambuf_iterator<char>() };
    auto http_server = lake::http::Server(&lakeapi::router::g_default_router);
    // Governator is a special WorkerGroup that controls Lake instance
	auto *gov = new lake::WorkerGroup(gov_script, true);
    try {
        http_server.Start(boost::asio::ip::make_address("0.0.0.0"), static_cast<unsigned short>(std::atoi(argv[2])));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
