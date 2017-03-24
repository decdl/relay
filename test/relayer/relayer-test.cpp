
// using relayer for proxy

#include <iostream>
#include <cstdlib>

#include "../../relayer.h"

namespace asio = boost::asio;
using asio::ip::tcp;

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0]
			<< " <local-port> <remote-host> <remote-port>" << std::endl;
		return EXIT_SUCCESS;
	}

	asio::io_service ios;
	tcp::socket socket_local(ios);
	tcp::socket socket_remote(ios);
	try
	{
		// establish connection
		{
			tcp::endpoint end_local = *tcp::resolver(ios).resolve(tcp::resolver::query(
											tcp::v4(), argv[1],
											tcp::resolver::query::passive));
			tcp::endpoint end_remote = *tcp::resolver(ios).resolve(tcp::resolver::query(
											tcp::v4(), argv[2], argv[3],
											tcp::resolver::query::passive));
			tcp::acceptor acceptor(ios, end_local);
			acceptor.accept(socket_local);
			socket_remote.connect(end_remote);
		}
		// send some data
		{
			static const char test_data[] = "test data\n";
			asio::write(socket_remote, asio::buffer(test_data, sizeof(test_data)));
		}
		// start relay
		relay::relayer relay(std::move(socket_remote), std::move(socket_local));
		ios.run();
		const auto result = relay.stat();
		std::cout << "in:\t" << result.first << std::endl
				  << "out:\t" << result.second << std::endl;
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "\e[31;1merror: \e[0m" << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
