
#include "prec.h"

void error(const std::string &str)
{
	std::cerr
#ifdef __linux__
		<< "\e[0;31;1m"
#endif // __linux__
		<< "error: "
#ifdef __linux__
		<< "\e[0m"
#endif // __linux__
		<< str << std::endl;
}

inline int print_usage(char *name)
{
	std::cerr << "Usage: " << name << " -s <remote-port> <local-port>" << std::endl
			  << "       " << name << " -c <remote-host> <remote-port> [<local-port>]" << std::endl
			  << "      For -s, <remote-port> and <local-port> must be different" << std::endl;
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	namespace asio = boost::asio;
	namespace ip = asio::ip;
	using ip::tcp;

	bool client_instance;

	if (argc < 1)
		return print_usage(argv[0]);
	if (std::strcmp(argv[1], "-s") == 0)
	{
		client_instance = false;
		if (argc != 4)
			return print_usage(argv[0]);
		if (std::strcmp(argv[2], argv[3]) == 0)
			return print_usage(argv[0]);
	}
	else if (std::strcmp(argv[1], "-c") == 0)
	{
		client_instance = true;
		if (argc != 4 && argc != 5)
			return print_usage(argv[0]);
	}
	else
		return print_usage(argv[0]);

	try
	{
		asio::io_service ios;
		std::unique_ptr<relay::client> client;
		std::unique_ptr<relay::server> server;
		if (client_instance)
			client = std::make_unique<relay::client>(ios);
		else
			server = std::make_unique<relay::server>(ios);
		// resolve hosts
		{
			tcp::resolver resolver(ios);
			typedef tcp::resolver::iterator iterator;
			if (client_instance)	// client mode
			{
				// resolve remote
				{
					tcp::resolver::query query(tcp::v4(), argv[2], argv[3]);
					iterator it = resolver.resolve(query);
					client->set_remote(*it);
				}
				// resolve local
				{
					tcp::resolver::query query(tcp::v4(), "localhost", argc==4 ? argv[3] : argv[4]);
					iterator it = resolver.resolve(query);
					client->set_local(*it);
				}
			}
			else					// server mode
			{
				// resolve remote
				{
					tcp::resolver::query query(tcp::v4(), "localhost", argv[2]);
					iterator it = resolver.resolve(query);
					server->set_remote(*it);
				}
				// resolve local
				{
					tcp::resolver::query query(tcp::v4(), "localhost", argv[3]);
					iterator it = resolver.resolve(query);
					server->set_local(*it);
				}
			}
		}
		// start relay
		if (client_instance)
			client->connect();
		else
			server->start();
		ios.run();
		// print statistics
		std::pair<size_t, size_t> stat = client_instance ? client->stat() : server->stat();
		std::cout << "bytes in:\t" << stat.first << std::endl
				  << "bytes out:\t" << stat.second << std::endl;
		return EXIT_SUCCESS;
	}
	catch (const boost::system::system_error &e)
	{
		error(e.what());
		return EXIT_FAILURE;
	}
	catch (const std::runtime_error &e)
	{
		error(e.what());
		return EXIT_FAILURE;
	}
}
