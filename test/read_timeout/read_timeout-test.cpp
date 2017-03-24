
// read_timeout client

#include <iostream>
#include <cstdlib>

#include "../../relayer.h"

namespace asio = boost::asio;
using asio::ip::tcp;

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0]
			<< " <server-host> <server-port>" << std::endl;
		return EXIT_SUCCESS;
	}

	asio::io_service ios;
	tcp::socket socket(ios);
	try
	{
		// establish connection
		{
			tcp::endpoint endpoint = *tcp::resolver(ios).resolve(tcp::resolver::query(
										tcp::v4(), argv[1], argv[2]));
			socket.connect(endpoint);
		}
		relay::buffer_t buffer(4096);
		for (size_t t = 1<<16; t != 0; t >>= 1)
		{
			std::cout << "timeout: " << t << "\tms\t";
			try
			{
				relay::read_timeout(socket, buffer, 4096, t);
				std::cout << "\e[32mpassed\e[0m" << std::endl;
			}
			catch (const boost::system::system_error &e)
			{
				if (e.code() == asio::error::eof)
					std::cout << "\e[31mtime out\e[0m" << std::endl;
				else
					throw;
			}
		}
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "\e[31;1merror: \e[0m" << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
