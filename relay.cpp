
#include "prec.h"

namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;

const uint8_t cc_sig[] = {0xe9, 0xfd, 0xfc, 0xab, 0x14, 0xf9, 0x44, 0x95};
const uint8_t ss_sig[] = {0x85, 0xf4, 0x1a, 0x78, 0xe5, 0xc9, 0x53, 0x0e};
const size_t size_cc_sig = sizeof(cc_sig);
const size_t size_ss_sig = sizeof(ss_sig);

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

int main(int argc, char **argv)
{
	if (argc < 3 || 4 < argc)
	{
		std::cerr << "Usage: " << argv[0] << " <remote-host> <remote-port> [<local-port>]" << std::endl;
		return 0;
	}

	try
	{
		asio::io_service ios;
		
		relay_cc cc(ios);

		// resolve hosts
		{
			tcp::resolver resolver(ios);
			typedef tcp::resolver::iterator iterator;
			const iterator end;
			// resolve remote
			{
				tcp::resolver::query query(tcp::v4(), argv[1], argv[2]);
				iterator it = resolver.resolve(query);
				cc.set_remote(*it);
			}
			// resolve local
			{
				tcp::resolver::query query(tcp::v4(), "localhost", argc==4 ? argv[3] : argv[2]);
				iterator it = resolver.resolve(query);
				cc.set_local(*it);
			}
		}

		cc.connect();
		ios.run();

		std::pair<size_t, size_t> stat = cc.stat();
		std::cout << "bytes in:\t" << stat.first << std::endl
				  << "bytes out:\t" << stat.second << std::endl;
		return 0;
	}
	catch (const failure &f)
	{
		error(f.what());
		return 1;
	}
	catch (const boost::system::system_error &e)
	{
		error(e.what());
		return -1;
	}
	return 0;
}
