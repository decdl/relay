
#include "prec.h"

namespace relay
{
	namespace asio = boost::asio;
	namespace ip = asio::ip;
	using ip::tcp;

	const uint8_t client_sig[] = {0xe9, 0xfd, 0xfc, 0xab, 0x14, 0xf9, 0x44, 0x95};
	const uint8_t server_sig[] = {0x85, 0xf4, 0x1a, 0x78, 0xe5, 0xc9, 0x53, 0x0e};
	const size_t size_client_sig = sizeof(client_sig);
	const size_t size_server_sig = sizeof(server_sig);

	void read_timeout(tcp::socket &socket, void *buffer, size_t size, size_t milliseconds)
	{
		bool timeout = false;
		asio::steady_timer timer(socket.get_io_service());
		timer.expires_from_now(std::chrono::milliseconds(milliseconds));
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
		socket.get_io_service().reset();
		if (timeout)
			throw boost::system::system_error(asio::error::eof);
	}

	// buffer_t functions
	
	// constructor
	buffer_t::buffer_t(size_t size)
	{
		data = std::make_unique<uint8_t[]>(size);
	}

	// converter
	buffer_t::operator void*() const
	{
		return data.get();
	}

	// offset dereference
	uint8_t buffer_t::operator[](size_t offset) const
	{
		return data[offset];
	}
	uint8_t & buffer_t::operator[](size_t offset)
	{
		return data[offset];
	}
}

