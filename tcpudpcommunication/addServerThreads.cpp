#include <iostream>
#include <errno.h>
#include <cstring>
#include <fstream>
#include "monitor.h"
#include <vector>

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
#define BACKLOG 10

///////////////////////////////////////////////////////
struct Dns
{
std::string name;
std::string ipv4;
};
///////////////////////////////////////////////////////


class Note//utworzenie klasy o i zadeklarowanie jej
{
std::vector<Dns> dns = {{"a","1"},{"b","2"},{"c","3"},{"d","4"}};

Semaphore mutex;

public:
	Note() : mutex(1)
	{
	}

std::string cry(std::string name)
{

	for(int i=0;i<dns.size();++i)
	{
		if( name == dns[i].name )
		{
			return dns[i].ipv4;
		}
	}
	return "brak";
}

std::string add_save(std::string name, std::string ipv4)
{
	std::cout << " add_save\n";
	mutex.p();



	

	for(int i=0;i<dns.size();++i)
	{
		if( name == dns[i].name )
		{		
			dns[i].ipv4 = ipv4;
			mutex.v();///
				std::cout << " znaleziono\n";
			return "Zmodyfikowano ipv4";
		}

	}

	for(int i=0;i<dns.size();++i)
	{
		if( ipv4 == dns[i].ipv4 )
		{		
			dns[i].name = name;
			mutex.v();///
	

			return "Zmodyfikowano nazwe";
		}

	}
	Dns temp={name, ipv4};
	dns.push_back(temp);
		
	mutex.v();

	return "Wykonano zapis";
	
}
};


Note note;

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
	
	int i;
	char a[50], b[20];
	
	do
	{
		if (read(clientSocket, (char *)(&i), sizeof(i)) == -1)
		{
			std::cout << "Error recv.\n";
			return;
		}
	//switch
	switch(i)
	{
		case 1:
			{
	
		
				if (read(clientSocket, (char *)(&a), sizeof(a)) == -1)
				{
					std::cout << "Error recv.\n";
					return;
				}
				std::cout << "Received a= " << a << ".\n";


				if (read(clientSocket, (char *)(&b), sizeof(b)) == -1)// recv
				{
					std::cout << "Error recv.\n";
					return;
				}

				std::cout << "Received b= " << b << ".\n";

				char kelp[30];		

				strcpy(kelp,note.add_save(a,b).c_str());
				
				if (send(clientSocket, (char *)(&kelp), sizeof(kelp), 0) == -1)//to nie
				{
					std::cout << "Error send.\n";
					return;
				}

			
				break;
			}
		case 2:
			{
	
				if (read(clientSocket, (char *)(&a), sizeof(a)) == -1)
				{
					std::cout << "Error recv.\n";
					return;
				}
				std::cout << "Received a= " << a << ".\n";

				strcpy(b,note.cry(a).c_str());

				if (send(clientSocket, (char *)(&b), sizeof(b), 0) == -1)//to nie
				{
					std::cout << "Error send.\n";
					return;
				}
			break;
			}
		case 3:
			{

			exit(0);

			}

	}
	}while(true);

}


int main(void)
{

	Note note;
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

