#include <iostream>
#include <errno.h>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif


#define PORT "5101"
#define MAXDATASIZE 100



void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << "\n";
		return 1;
	}
#endif
	if (argc != 2)
	{
		std::cout << "Usage: addClient hostname\n";
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo* servinfo;
	int result;
	if ((result = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
	{
		std::cout << "getaddrinfo error: " << gai_strerror(result) << ".\n";
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	if (servinfo == nullptr)
	{
		std::cout << "Bad address.\n";
		freeaddrinfo(servinfo);
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	int clientSocket;
	if ((clientSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		std::cout << "Error socket.\n";
		freeaddrinfo(servinfo);
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	if (connect(clientSocket, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
#ifdef _WIN32
		closesocket(clientSocket);
#else
		close(clientSocket);
#endif
		std::cout << "Error connect.\n";
		freeaddrinfo(servinfo);
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	
	char s[INET6_ADDRSTRLEN];
	inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr*)servinfo->ai_addr), s, sizeof s);
	std::cout << "Client connecting to " << s << "\n";

	freeaddrinfo(servinfo);
	
	
	int i;
	char a[50], b[20];
	char sum[30];

	std::cout << "Podaj opcje (1-dodanie/modyfikacja, 2-wyswietlenie danych, 3-kill server" << std::endl;

	while(true)
	{
	

		std::cin >> i;

		if(i >= 3 || i < 1)
		{
			if(i == 3)
				if (send(clientSocket, (char*)(&i), sizeof(i), 0) == -1)
				{
					std::cout << "Error recv.\n";
#ifdef _WIN32
					WSACleanup();
#endif
					return 1;
				}

			break;
		}

	
		if (send(clientSocket, (char *)(&i),sizeof(i), 0) == -1)
		{
			std::cout << "Error recv.\n";
#ifdef _WIN32
			WSACleanup();
#endif
			return 1;
		}


////////////////////////////////////////////////
		switch(i)
		{
			case 1:
			{
				std::cout << "wprowadz wartosci" << std::endl;

				std::cin >> a;
				std::cin >> b;

				if (send(clientSocket, (char *)(&a), sizeof(a), 0) == -1)
				{
					std::cout << "Error send.\n";
#ifdef _WIN32
					WSACleanup();
#endif
					return 1;
				}

				if (send(clientSocket, (char *)(&b), sizeof(b), 0) == -1)
				{
					std::cout << "Error send.\n";
#ifdef _WIN32
					WSACleanup();
#endif
					return 1;
				}
				
					if (recv(clientSocket, (char *)(&sum), sizeof(sum),0) == -1)
				{
					std::cout << "Error recv.\n";
#ifdef _WIN32
					WSACleanup();
#endif
					return 1;
				}

				std::cout << "Client received: " << sum << "\n";

				std::cout << "Podaj opcje (1-dodanie/modyfikacja, 2-wyswietlenie danych, 3-kill server" << std::endl;
			
				break;
			}

			case 2:
			{
				std::cout << "wprowadz nazwe strony" << std::endl;

	
	
				std::cin>>a;
				std::cout << a;
	
				if (send(clientSocket, (char *)(&a), sizeof(a), 0) == -1)
				{
					std::cout << "Error send.\n";
#ifdef _WIN32	
					WSACleanup();
#endif
					return 1;
				}

				
				
				if (recv(clientSocket, (char *)(&sum), sizeof(sum),0) == -1)
				{
					std::cout << "Error recv.\n";
#ifdef _WIN32
					WSACleanup();
#endif
					return 1;
				}

				std::cout << "Client received: " << sum << "\n";

				std::cout << "Podaj opcje (1-dodanie/modyfikacja, 2-wyswietlenie danych, 3-kill server" << std::endl;


				break;
			}
		}
	}

#ifdef _WIN32
	closesocket(clientSocket);
#else
	close(clientSocket);
#endif

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
