
#include "prec.h"

namespace relay
{

	namespace asio = boost::asio;
	namespace ip = asio::ip;
	using ip::tcp;

	// constructor
	client::client(asio::io_service &io_service, size_t size)
		: ios(io_service), remote_set(false), local_set(false), buffer_size(size) {}

	// set remote endpoint
	void client::set_remote(const tcp::endpoint &remote) noexcept
	{
		endpoint_remote = remote;
		remote_set = true;
	}

	// set local endpoint
	void client::set_local(const tcp::endpoint &local) noexcept
	{
		endpoint_local = local;
		local_set = true;
	}

	// statistics
	std::pair<size_t, size_t> client::stat() const
	{
		if (!relay)
			throw std::runtime_error("no relayer object");
		return relay->stat();
	}

	// start connection
	void client::connect()
	{
		if (!remote_set || !local_set)
			throw std::runtime_error("endpoints not set");

		tcp::socket socket_remote(ios);
		tcp::socket socket_local(ios);
		buffer_t buffer(size_server_sig);
		// establish remote connection
		socket_remote.connect(endpoint_remote);
		try
		{
			asio::write(socket_remote, asio::buffer(client_sig, size_client_sig));
			read_timeout(socket_remote, buffer, size_server_sig);
			if (std::memcmp(buffer, server_sig, size_server_sig) != 0)
				throw std::runtime_error("remote server signature verification failed");
		}
		catch (const boost::system::system_error &e)
		{
			if (e.code() == asio::error::broken_pipe
					|| e.code() == asio::error::eof)
				throw std::runtime_error("remote server connection reset");
			throw;
		}
retry:
		try
		{
			// waiting for connection
			asio::read(socket_remote, asio::buffer(buffer, 1));
			// establish local connection
			socket_local.connect(endpoint_local);
		}
		catch (const boost::system::system_error &e)
		{
			if (e.code() == asio::error::connection_refused)
			{
				buffer[0] = 1;
				asio::write(socket_remote, asio::buffer(buffer, 1));
				goto retry;
			}
			if (e.code() == asio::error::eof)
				throw std::runtime_error("remote server connection reset");
			throw;
		}
		buffer[0] = 0;
		asio::write(socket_remote, asio::buffer(buffer, 1));
		// start relay
		relay = std::make_unique<relayer>(std::move(socket_remote), std::move(socket_local), buffer_size);
	}
}

