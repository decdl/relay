
#ifndef _RELAY_CC_H_
#define _RELAY_CC_H_

#include <exception>
#include <string>
#include <functional>
#include <utility>

#include <boost/asio.hpp>

#include "sig.h"

class failure : std::exception
{
	public:
		failure(const std::string &s);
		virtual const char * what() const noexcept;
	private:
		std::string msg;
};

namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;

class relay_cc
{
	public:
		// constructor
		relay_cc(asio::io_service &ios, size_t buffer_size = 8192);
		relay_cc(const relay_cc &) = delete;

		// copy assignment
		void operator=(const relay_cc &) = delete;

		// destructor
		~relay_cc();

		// set remote endpoint
		void set_remote(const tcp::endpoint &remote);

		// set local endpoint
		void set_local(const tcp::endpoint &local);

		// connect
		void connect();

		// stat
		std::pair<size_t, size_t> stat() const;

	private:
		tcp::socket socket_remote;
		tcp::socket socket_local;
		tcp::endpoint endpoint_remote;
		tcp::endpoint endpoint_local;
		bool remote_set, local_set;
		size_t buffer_size;
		void *buffer_remote;
		void *buffer_local;
		size_t bytes_in, bytes_out;

		static void read_timeout(tcp::socket &socket, void *buffer, size_t size);

		void handle_remote_read(const boost::system::error_code &ec, size_t bytes_transferred);
		void handle_remote_write(const boost::system::error_code &ec, size_t bytes_transferred);
		void handle_local_read(const boost::system::error_code &ec, size_t bytes_transferred);
		void handle_local_write(const boost::system::error_code &ec, size_t bytes_transferred);
};

#endif
