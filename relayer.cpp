
#include "prec.h"

namespace relay
{
	namespace asio = boost::asio;
	using ip::tcp;
	using namespace std::placeholders;

	template<typename T>
	static inline T & select(bool b, T &x, T &y)
	{
		if (b)
			return x;
		else
			return y;
	}

	// constructor
	relayer::relayer(tcp::socket && A, tcp::socket && B, size_t size) : buffer_size(size),
																		// 0 for normal, 1 for closing, 2 for closed
																		shutdown_AB(0), shutdown_BA(0),
																		socketA(std::forward<tcp::socket>(A)),
																		socketB(std::forward<tcp::socket>(B)),
																		writingA(false), writingB(false),
																		bytesA(0), bytesB(0)
	{
		// check argument
		if (buffer_size == 0)
			throw std::invalid_argument("zero relay buffer size");
		if (!socketA.is_open() || !socketB.is_open())
			throw std::invalid_argument("sockets not open");
		// start relay
		std::shared_ptr<buffer_t> bufferA = std::make_shared<buffer_t>(buffer_size);
		std::shared_ptr<buffer_t> bufferB = std::make_shared<buffer_t>(buffer_size);
		socketA.async_read_some(asio::buffer(*bufferA, buffer_size),
				std::bind(&relayer::handle_read, this, bufferA, true, _1, _2));
		socketB.async_read_some(asio::buffer(*bufferB, buffer_size),
				std::bind(&relayer::handle_read, this, bufferB, false, _1, _2));
	}

	// destructor
	relayer::~relayer()
	{
		while (dataA.size() > 0)
			dataA.pop();
		while (dataB.size() > 0)
			dataB.pop();
	}

	// statistics
	std::pair<size_t, size_t> relayer::stat() const noexcept
	{
		return std::make_pair(bytesA, bytesB);
	}

	// async read handler
	void relayer::handle_read(std::shared_ptr<buffer_t> buffer, bool from_A,
							  const boost::system::error_code &ec, size_t bytes_transferred)
	{
		// error checking
		if (ec)
		{
			if (ec == asio::error::eof)
			{
				// handle eof
				select(from_A, shutdown_AB, shutdown_BA) = 1;
				if (bytes_transferred != 0)
					select(from_A, dataA, dataB).push(std::make_pair(buffer, bytes_transferred));
				check_and_write();
				return;
			}
			else
				throw boost::system::system_error(ec);
		}
		// signal new data to write
		if (bytes_transferred != 0)
		{
			select(from_A, dataA, dataB).push(std::make_pair(buffer, bytes_transferred));
			check_and_write();
		}
		// read new data
		if (select(from_A, shutdown_AB, shutdown_BA) == 0)
		{
			buffer = std::make_shared<buffer_t>(buffer_size);
			select(from_A, socketA, socketB).async_read_some(asio::buffer(*buffer, bytes_transferred),
					std::bind(&relayer::handle_read, this, buffer, from_A, _1, _2));
		}
	}

	// async write handler
	void relayer::handle_write(std::shared_ptr<buffer_t> buffer, bool to_A,
							   const boost::system::error_code &ec, size_t bytes_transferred)
	{
		// error checking
		if (ec)
		{
			if (ec == asio::error::broken_pipe)
			{
				// handle broken pipe
				if (select(to_A, shutdown_BA, shutdown_AB) != 2)
				{
					select(to_A, socketB, socketA).shutdown(tcp::socket::shutdown_both/*recieve*/);
					select(to_A, shutdown_BA, shutdown_AB) = 2;
				}
				while (select(to_A, dataB, dataA).size() > 0)
					select(to_A, dataB, dataA).pop();
				return;
			}
			else
				throw boost::system::system_error(ec);
		}
		// unblock
		select(to_A, writingA, writingB) = false;
		select(to_A, bytesB, bytesA) += bytes_transferred;
		// continue next write
		check_and_write();
	}

	// check and write
	void relayer::check_and_write()
	{
		if (!writingA && shutdown_BA != 2)
		{
			if (dataB.size() > 0)
			{
				std::pair<std::shared_ptr<buffer_t>, size_t> buffer = dataB.front();
				dataB.pop();
				writingA = true;
				asio::async_write(socketA, asio::buffer(*buffer.first, buffer.second),
						std::bind(&relayer::handle_write, this, buffer.first, true, _1, _2));
			}
			else if (shutdown_BA == 1)
			{
				socketA.shutdown(tcp::socket::shutdown_both/*send*/);
				shutdown_BA = 2;
			}
		}
		if (!writingB && shutdown_AB != 2)
		{
			if (dataA.size() > 0)
			{
				std::pair<std::shared_ptr<buffer_t>, size_t> buffer = dataA.front();
				dataA.pop();
				writingB = true;
				asio::async_write(socketB, asio::buffer(*buffer.first, buffer.second),
						std::bind(&relayer::handle_write, this, buffer.first, false, _1, _2));
			}
			else if (shutdown_AB == 1)
			{
				socketB.shutdown(tcp::socket::shutdown_both/*send*/);
				shutdown_AB = 2;
			}
		}
	}
}
