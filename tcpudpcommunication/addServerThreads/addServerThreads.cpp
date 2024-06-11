#include <iostream>
#include <errno.h>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h> 
#include <ws2tcpip.h>
#include <fstream>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif



#define PORT "5100"
#define BACKLOG 10

void client(int clientSocket);


void createThread(void* threadProd(void*), int parameter)
{
#ifdef _WIN32
	HANDLE tid;
	DWORD id;

	// utworzenie w¹tków
	tid = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProd, (void*)parameter, 0, &id);
#else
	pthread_t tid;
	// utworzenie w¹tków
	pthread_create(&tid, NULL, threadProd, (void*)(long)parameter);
#endif
}

void save(std::string napis)
{
	std::fstream plik;
	plik.open("dane.txt", std::ios::in | std::ios::out);
	if (plik.good() == true)
	{
		plik << napis;
		plik.close();
	}
	
}

void* threadClient(void* arg)
{
	client((long)arg);
	return NULL;
}

void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void client(int clientSocket)
{

	//TODO
	std::string a , b ; // ipv4 clienta i strona np.www.google.pl zapisac to w pliku tekstowym 1234.1234.1234.1234;www.marekjamusz.pl
	if (recv(clientSocket, (char*)(&a), sizeof(a), 0) == -1)
	{
		std::cout << "Error recv.\n";
		return;
	}
	struct sockaddr_in their_addr; // informacja o adresie osoby ³¹cz¹cej siê
	 a = (std::string)inet_ntoa(their_addr.sin_addr);

	if (recv(clientSocket, (char*)(&b), sizeof(b), 0) == -1)
	{
		std::cout << "Error recv.\n";
		return;
	}

	std::cout << "Received b= " << b << ".\n";

	std::string sum = a + ";" + b;

	std::cout << "Send sum = " << sum << ".\n";

	if (&sum != nullptr)
	{
		std::cout << "Error send.\n";
		return;
	}

	
	if (send(clientSocket, (char*)(&sum), sizeof(sum), 0) == -1)
	{
		std::cout << "Error send.\n";
		return;
	}
}


int main(void)
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

	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* servinfo;
	int result;
	if ((result = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
	{
		std::cout << "retaddrinfo error: " << gai_strerror(result) << ".\n";
		return 1;
	}

	if (servinfo == nullptr)
	{
		std::cout << "No interface to listen.\n";
		return 1;
	}

	int listenSocket;
	if ((listenSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		freeaddrinfo(servinfo);
		std::cout << "Error creatind socket.\n";
		return 1;
	}

	int yes = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)(&yes), sizeof(int)) == -1)
	{
		freeaddrinfo(servinfo);
		std::cout << "Error setsockopt.\n";
		return 1;
	}

	if (bind(listenSocket, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		freeaddrinfo(servinfo);
#ifdef _WIN32
		closesocket(listenSocket);
#else
		close(listenSocket);
#endif
		std::cout << "Error bind.";
		return 1;
	}

	freeaddrinfo(servinfo);

	if (listen(listenSocket, BACKLOG) == -1)
	{
		std::cout << "Error listen.\n";
		return 1;
	}

	std::cout << "Server waiting for connections.\n";

	while (true)
	{
		struct sockaddr_storage their_addr = {};
		socklen_t sin_size = sizeof(their_addr);
		int clientSocket;
		clientSocket = accept(listenSocket, (struct sockaddr*)&their_addr, &sin_size);
		if (clientSocket == -1)
			std::cout << "Error accept.";
		else
		{
			char s[INET6_ADDRSTRLEN];
			inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
			std::cout << "Server got connection from " << s << ".\n";

			createThread(threadClient, clientSocket);
		}
	}
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

