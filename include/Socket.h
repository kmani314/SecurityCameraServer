#ifndef SOCKET_H
#define SOCKET_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <sys/types.h>
#include "unistd.h"
#include <string>

typedef struct sockaddr_in sockaddr_in;

class Socket {
	private:

	int socketOption = 1;
	
	struct sockaddr_in address;
	
	public:
	int descriptor;
	int connectedSocket = -1;
	Socket();
	~Socket();
	Socket(int);
	void listen(int, int);
	void connect(int, std::string);
	
	void waitForConnection();

	int write(void*, int);
	int read(void*, int);
};

#endif
