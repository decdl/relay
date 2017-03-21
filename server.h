
#ifndef _RELAY_SERVER_H_
#define _RELAY_SERVER_H_

#include <memory>

#include <boost/asio.hpp>

namespace relay
{
	namespace asio = boost::asio;
	namespace ip = asio::ip;
	using ip::tcp;

	class server
	{
		public:
			// constructor
			server(asio::io_service &io_service, size_t size = 8192);
			server(const server &) = delete;

			// copy assignment
			void operator=(const server &) = delete;

			// set remote acceptor port
			void set_remote(const tcp::endpoint &remote) noexcept;

			// set local acceptor port
			void set_local(const tcp::endpoint &local) noexcept;

			// start listening
			void start();

			// statistics
			std::pair<size_t, size_t> stat() const;

		private:
			asio::io_service &ios;
			tcp::endpoint endpoint_remote;
			tcp::endpoint endpoint_local;
			bool remote_set, local_set;
			size_t buffer_size;
			std::unique_ptr<relayer> relay;
	};
}

#endif
