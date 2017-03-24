
#ifndef _RELAY_CLIENT_H_
#define _RELAY_CLIENT_H_

#include <memory>

#include <boost/asio.hpp>

namespace relay
{
	namespace asio = boost::asio;
	namespace ip = asio::ip;
	using ip::tcp;

	class client
	{
		public:
			// constructor
			client(asio::io_service &io_service, size_t size = 4096);
			client(const client &) = delete;

			// copy assignment
			void operator=(const client &) = delete;

			// set remote endpoint
			void set_remote(const tcp::endpoint &remote) noexcept;

			// set local endpoint
			void set_local(const tcp::endpoint &local) noexcept;

			// start connection
			void connect();

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
