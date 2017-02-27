
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
	int sockfd = -1;

	try
	{
		if (argc < 3)
		{
			std::wcerr << L"usage: client <hostname> <port>" << std::endl;
			return 0;
		}
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			throw L"Cannot opening socket";
		hostent *server = gethostbyname(argv[1]);
		if (server == nullptr)
			throw "Failed to retrive host";

		sockaddr_in serv_addr;
		std::memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		serv_addr.sin_port = htons(atoi(argv[2]));
		if (connect(sockfd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
			throw L"Failed to connect";

		while (true)
		{
			static uint8_t buffer[256];
			wchar_t *buf = reinterpret_cast<wchar_t *>(buffer);

			auto check = [buf, sockfd]()->bool
			{
				if (buf[0] == L'\0')
				{
					if (sockfd >= 0)
						close(sockfd);
					return true;
				}
				return false;
			};

			size_t len = 0;
			std::wcin.getline(buf, sizeof(buffer) / sizeof(wchar_t));
			if (!std::wcin.eof())
			{
				len = std::wcslen(buf);
				buf[len++] = L'\n';
			}
			buf[len++] = L'\0';
			if (write(sockfd, buffer, len * sizeof(wchar_t)) < 0)
				throw L"Failed to write to socket";
			if (check())
				return 0;
			if (read(sockfd, buffer, 256) < 0)
				throw L"Failed to read from socket";
			if (check())
				return 0;
			std::wcout << buf;
		}
	}
	catch (const wchar_t *error)
	{
		std::wcerr << L"\e[31m" << error << L"\e[0m" << std::endl;
		if (sockfd >= 0)
			close(sockfd);
		return -1;
	}
}

