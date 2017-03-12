
#include "prec.h"

namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;

failure::failure(const std::string &s) : msg(s) {}
const char * failure::what() const noexcept
{
	return msg.c_str();
}

relay_cc::relay_cc(asio::io_service &ios, size_t size) : socket_remote(ios),
														 socket_local(ios),
														 remote_set(false),
														 local_set(false),
														 buffer_size(size),
														 bytes_in(0), bytes_out(0)
{
	if (buffer_size < size_ss_sig)
		buffer_size = size_ss_sig;
	if (buffer_size < size_cc_sig)
		buffer_size = size_cc_sig;
	buffer_remote = operator new(size);
	buffer_local = operator new(size);
}

relay_cc::~relay_cc()
{
	operator delete(buffer_remote);
	operator delete(buffer_local);
}

void relay_cc::set_remote(const tcp::endpoint &remote)
{
	endpoint_remote = remote;
	remote_set = true;
}

void relay_cc::set_local(const tcp::endpoint &local)
{
	endpoint_local = local;
	local_set = true;
}

void relay_cc::read_timeout(tcp::socket &socket, void *buffer, size_t size)
{
	bool timeout = false;
	asio::steady_timer timer(socket.get_io_service());
	timer.expires_from_now(std::chrono::milliseconds(1500));
	timer.async_wait(
			[&socket](const boost::system::error_code &ec)->void
			{
				if (ec)
				{
					if (ec != asio::error::operation_aborted)
						throw boost::system::system_error(ec);
				}
				else
					socket.cancel();
			});
	asio::async_read(
			socket,
			asio::buffer(buffer, size),
			[&timer, &timeout](const boost::system::error_code &ec, size_t bytes_transferred)->void
			{
				if (ec)
					if (ec == asio::error::operation_aborted)
						timeout = true;
					else
						throw boost::system::system_error(ec);
				else
					timer.cancel();
			});
	socket.get_io_service().run();
	if (timeout)
		throw boost::system::system_error(asio::error::eof);
}

void relay_cc::connect()
{
	if (!remote_set || !local_set)
		throw failure("endpoints not set");

	// establish connection
	socket_remote.connect(endpoint_remote);
	try
	{
		asio::write(socket_remote, asio::buffer(cc_sig, size_cc_sig));
		read_timeout(socket_remote, buffer_remote, size_ss_sig);
		if (std::memcmp(buffer_remote, ss_sig, size_ss_sig) != 0)
			throw failure("remote server signature verification failed");
		asio::read(socket_remote, asio::buffer(buffer_remote, 1));
	}
	catch (const boost::system::system_error &e)
	{
		if (e.code() == asio::error::broken_pipe)
			throw failure("remote server connection reset");
		if (e.code() == asio::error::eof)
			throw failure("cannot verify remote server signature");
		throw;
	}
	socket_local.connect(endpoint_local);

	// start proxy
	socket_remote.async_read_some(asio::buffer(buffer_remote, buffer_size),
			std::bind(&relay_cc::handle_remote_read, this, std::placeholders::_1, std::placeholders::_2));
	socket_local.async_read_some(asio::buffer(buffer_local, buffer_size),
			std::bind(&relay_cc::handle_local_read, this, std::placeholders::_1, std::placeholders::_2));
}

void relay_cc::handle_remote_read(const boost::system::error_code &ec, size_t bytes_transferred)
{
	if (ec)
	{
		if (ec == asio::error::eof)
			remote_set = false;
		else
			throw boost::system::system_error(ec);
	}
	asio::async_write(socket_local, asio::buffer(buffer_remote, bytes_transferred),
			std::bind(&relay_cc::handle_local_write, this, std::placeholders::_1, std::placeholders::_2));
}

void relay_cc::handle_local_read(const boost::system::error_code &ec, size_t bytes_transferred)
{
	if (ec)
	{
		if (ec == asio::error::eof)
			local_set = false;
		else
			throw boost::system::system_error(ec);
	}
	asio::async_write(socket_remote, asio::buffer(buffer_local, bytes_transferred),
			std::bind(&relay_cc::handle_remote_write, this, std::placeholders::_1, std::placeholders::_2));
}

void relay_cc::handle_remote_write(const boost::system::error_code &ec, size_t bytes_transferred)
{
	if (ec)
		throw boost::system::system_error(ec);
	bytes_out += bytes_transferred;
	if (local_set)
		socket_local.async_read_some(asio::buffer(buffer_local, buffer_size),
				std::bind(&relay_cc::handle_local_read, this, std::placeholders::_1, std::placeholders::_2));
}

void relay_cc::handle_local_write(const boost::system::error_code &ec, size_t bytes_transferred)
{
	if (ec)
		throw boost::system::system_error(ec);
	bytes_in += bytes_transferred;
	if (remote_set)
		socket_remote.async_read_some(asio::buffer(buffer_remote, buffer_size),
				std::bind(&relay_cc::handle_remote_read, this, std::placeholders::_1, std::placeholders::_2));
}

std::pair<size_t, size_t> relay_cc::stat() const
{
	return std::pair<size_t, size_t>(bytes_in, bytes_out);
}
