
#include <iostream>
#include <string>
#include <boost/asio.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: server <port>" << std::endl;
		return 0;
	}

	// data
	static uint8_t buf[1024];
	std::vector<char> response;
	size_t remain = 0;

	// i/o objects
	boost::asio::io_service ios;
	tcp::acceptor acceptor(ios);
	tcp::socket socket(ios);
	
	try
	{
		tcp::endpoint end = *tcp::resolver(ios).resolve(tcp::resolver::query(
					tcp::v4(),
					argv[1],
					tcp::resolver::query::numeric_service | tcp::resolver::query::passive));
		acceptor.open(end.protocol());
		acceptor.bind(end);
		acceptor.listen();
		acceptor.accept(socket);
		while (true)
		{
			size_t n = socket.read_some(asio::buffer(buf, sizeof(buf)));
			for (size_t i = 0; i < n; i++)
				if (remain == 0)
				{
					remain = *reinterpret_cast<size_t*>(buf + i);
					i += sizeof(size_t) - 1;
				}
				else
				{
					if ('a' <= buf[i] && buf[i] <= 'z')
						buf[i] += 'A' - 'a';
					else if ('A' <= buf[i] && buf[i] <= 'Z')
						buf[i] += 'a' - 'A';
					response.push_back(buf[i]);
					if (--remain == 0)
					{
						socket.send(asio::buffer(response));
						response.clear();
					}
				}
		}
	}
	catch (const boost::system::system_error &e)
	{
		if (e.code() != asio::error::eof)
		{
			std::cerr << "\e[31mError: " << e.what() << "\e[0m" << std::endl;
			return -1;
		}
	}
	// finish the last segment
	socket.send(asio::buffer(response));
	return 0;
}
