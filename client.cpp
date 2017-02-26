
#include <iostream>
#include <boost/asio.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: client <host> <port>" << std::endl;
		return 0;
	}

	try
	{
		asio::io_service ios;
		tcp::socket socket(ios);
		socket.connect(*tcp::resolver(ios).resolve(tcp::resolver::query(
						tcp::v4(),
						argv[1],
						argv[2],
						tcp::resolver::query::numeric_service)));
		std::string msg;
		std::cout << "\e[0;35m";
		static char buf[1025] = {'\0'};
		while (true)
		{
			std::string str;
			if (std::getline(std::cin, str).eof()) break;
			if (str.empty())
			{
				size_t size = msg.size();
				asio::write(socket, asio::buffer(&size, sizeof(size_t)));
				asio::write(socket, asio::buffer(msg));
				std::cout << "\e[0;32m";
				while (size >= sizeof(buf) - 1)
				{
					asio::read(socket, asio::buffer(buf, sizeof(buf) - 1));
					size -= sizeof(buf) - 1;
					std::cout << buf;
				}
				asio::read(socket, asio::buffer(buf, size));
				buf[size] = '\0';
				std::cout << buf << std::endl << "\e[0;35m";
				msg.clear();
			}
			else
				msg += std::move(str) += '\n';
		}
		size_t size = msg.size();
		asio::write(socket, asio::buffer(&size, sizeof(size_t)));
		asio::write(socket, asio::buffer(msg));
		std::cout << "\e[0;32m";
		while (size >= sizeof(buf) - 1)
		{
			asio::read(socket, asio::buffer(buf, sizeof(buf) - 1));
			size -= sizeof(buf) - 1;
			std::cout << buf;
		}
		asio::read(socket, asio::buffer(buf, size));
		buf[size] = '\0';
		std::cout << buf << std::endl;
	}
	catch (const boost::system::system_error &e)
	{
		if (e.code() != asio::error::broken_pipe
				&& e.code() != asio::error::eof)
		{
			std::cerr << "\e[0;31mError: " << e.what() << "\e[0m" << std::endl;
			std::cout << "\e[0m";
			return -1;
		}
	}
	std::cout << "\e[0m";
	return 0;
}
