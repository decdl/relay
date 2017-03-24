
#include "prec.h"

namespace relay
{
	namespace asio = boost::asio;
	namespace ip = asio::ip;
	using ip::tcp;

	// constructor
	server::server(asio::io_service &io_service, size_t size)
		: ios(io_service), remote_set(false), local_set(false), buffer_size(size) {}

	// set remote acceptor port
	void server::set_remote(const tcp::endpoint &remote) noexcept
	{
		endpoint_remote = remote;
		remote_set = true;
	}

	// set local acceptor port
	void server::set_local(const tcp::endpoint &local) noexcept
	{
		endpoint_local = local;
		local_set = true;
	}

	// statistics
	std::pair<size_t, size_t> server::stat() const
	{
		if (!relay)
			throw std::runtime_error("no relayer object");
		return relay->stat();
	}

	// start server
	void server::start()
	{
		if (!remote_set || ! local_set)
			throw std::runtime_error("endpoints not set");

		tcp::socket socket_remote(ios);
		tcp::socket socket_local(ios);
		buffer_t buffer(size_client_sig);
		// establish remote connection
		{
			tcp::acceptor acceptor(ios, endpoint_remote);
			acceptor.accept(socket_remote);
		}
		try
		{
			asio::write(socket_remote, asio::buffer(server_sig, size_server_sig));
			read_timeout(socket_remote, buffer, size_client_sig);
			if (std::memcmp(buffer, client_sig, size_client_sig) != 0)
				throw std::runtime_error("remote client signature verification failed");
		}
		catch (const boost::system::system_error &e)
		{
			if (e.code() == asio::error::broken_pipe
					|| e.code() == asio::error::eof)
				throw std::runtime_error("remote client connection reset");
			throw;
		}
retry:
		// establish local connection
		{
			tcp::acceptor acceptor(ios, endpoint_local);
			acceptor.accept(socket_local);
		}
		// signal client for the connection
		try
		{
			buffer[0] = 0;
			asio::write(socket_remote, asio::buffer(buffer, 1));
			read_timeout(socket_remote, buffer, 1, 10000);
			if (buffer[0] != 0)
			{
				socket_local.shutdown(tcp::socket::shutdown_both);
				socket_local.close();
				goto retry;
			}
		}
		catch (const boost::system::system_error &e)
		{
			if (e.code() == asio::error::eof)
				throw std::runtime_error("remote client connection reset");
			throw;
		}
		relay = std::make_unique<relayer>(std::move(socket_remote), std::move(socket_local), buffer_size);
	}
}
