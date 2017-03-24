
// read_timeout server

#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/asio.hpp>

#include "../../relay.h"

namespace asio = boost::asio;
using asio::ip::tcp;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0]
			<< " <server-port>" << std::endl;
		return EXIT_SUCCESS;
	}

	asio::io_service ios;
	tcp::socket socket(ios);
	try
	{
		// establish connection
		{
			tcp::endpoint endpoint = *tcp::resolver(ios).resolve(tcp::resolver::query(
										tcp::v4(), argv[1],
										tcp::resolver::query::passive));
			tcp::acceptor acceptor(ios, endpoint);
			acceptor.accept(socket);
		}
		relay::buffer_t buffer(4096);
		try
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				asio::write(socket, asio::buffer(buffer, 4096));
			}
		}
		catch (const boost::system::system_error &e)
		{
			if (e.code() != asio::error::broken_pipe)
				throw;
		}
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "\e[31;1merror: \e[0m" << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
