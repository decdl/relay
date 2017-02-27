
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	int sockfd = -1, newsockfd = -1;
	try
	{
		if (argc < 2)
		{
			std::wcerr << L"usage: server <port>" << std::endl;
			return 0;
		}
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			throw L"Cannot opening socket";

		sockaddr_in serv_addr, cli_addr;
		std::memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(atoi(argv[1]));

		if (bind(sockfd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
			throw L"Failed to bind";
		listen(sockfd, 5);
		socklen_t clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, reinterpret_cast<sockaddr *>(&cli_addr), &clilen);
		if (newsockfd < 0)
			throw L"Failed to accept";

		while (true)
		{
			static uint8_t buffer[256] = {0};
			wchar_t * buf = reinterpret_cast<wchar_t *>(buffer);

			auto check = [newsockfd, sockfd, buf]()->bool
			{
				if (buf[0] == L'\0')
				{
					close(newsockfd);
					close(sockfd);
					return true;
				}
				return false;
			};

			if (read(newsockfd, buffer, 256) < 0)
				throw L"Failed to read from socket";
			if (check())
				return 0;
			std::wcout << buf;
			std::wcin.getline(buf, sizeof(buffer) / sizeof(wchar_t));
			int len = 0;
			if (!std::wcin.eof())
			{
				len = std::wcslen(buf);
				buf[len++] = L'\n';
			}
			buf[len++] = L'\0';
			if (write(newsockfd, buf, len * sizeof(wchar_t)) < 0)
				throw L"Failed write to socket";
			if (check())
				return 0;
		}
	}
	catch (const wchar_t *error)
	{
		std::wcerr << L"\e[31m" << error << L"\e[0m" << std::endl;
		if (newsockfd >= 0)
			close(newsockfd);
		if (sockfd >= 0)
			close(sockfd);
		return -1;
	}
}

