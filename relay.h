
#ifndef _RELAY_H_
#define _RELAY_H_

#include <memory>

#include <boost/asio.hpp>

namespace relay
{
	namespace asio = boost::asio;
	namespace ip = asio::ip;
	using ip::tcp;

	extern const uint8_t client_sig[];
	extern const uint8_t server_sig[];
	extern const size_t size_client_sig;
	extern const size_t size_server_sig;

	// read data and throw eof if timeout
	void read_timeout(tcp::socket &socket, void *buffer, size_t size, size_t milliseconds = 1500);

	// buffer type
	class buffer_t
	{
		public:
			// constructor
			buffer_t(size_t size);
			buffer_t(const buffer_t &) = delete;

			// copy assignment
			void operator=(const buffer_t &) = delete;

			// convert to void pointer
			operator void*() const;

			// offset dereference
			uint8_t operator[](size_t offset) const;
			uint8_t & operator[](size_t offset);
		private:
			std::unique_ptr<uint8_t[]> data;
	};
}

#endif
