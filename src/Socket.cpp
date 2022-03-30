#include "Socket.h"
#include <cstring>

class SocketException : public std::exception {
	const char* msg;
	public:
	SocketException(const char* _msg) : msg(_msg) {}

	const char* what() const noexcept {
		return msg;
	}
};

Socket::Socket() {
	WSADATA wsa;
	SOCKET s;
	
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
		throw SocketException("Socket could not be created!"); // Some socket creation error
	}

	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}
}

Socket::~Socket() {
	close(connectedSocket);
	close(descriptor);
}
Socket::Socket(int connected) {
	descriptor = connectedSocket = connected;
}

void Socket::listen(int portNumber, int maxQueue) { // listen on port
	memset(&address, '0', sizeof(address));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portNumber);

	if(bind(descriptor, (struct sockaddr *)&address, sizeof(address)) < 0) throw SocketException("Socket could not bind");
	
	if(::listen(descriptor, maxQueue) < 0) throw SocketException("Socket could not listen.");
}

void Socket::connect(int portNumber, std::string hostAddr) { // connect to remote host
	memset(&address, '0', sizeof(address));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(portNumber); // network order

	//if(inet_pton(AF_INET, hostAddr.c_str(), &address.sin_addr) <= 0) throw SocketException("Connection Address Invalid.");

	if(::connect(descriptor, (struct sockaddr*)&address, sizeof(address)) < 0) throw SocketException("Failed to connect.");

}

void Socket::waitForConnection() { // block until incoming request - sets connectedSocket for reading and writing
	int addrlen = sizeof(address);
	connectedSocket = accept(descriptor, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	
	if(connectedSocket < 0) throw SocketException("Could not accept a valid socket.");	
}

int Socket::write(void* buf, int len) { // write len bytes from buf to socket
	int sent;
	
	if((sent = send(descriptor, (char*) buf, len, 0)) < 0) throw SocketException("Failed to send.");
	return sent; // return number of bytes sent
}

int Socket::read(void* buf, int len) { // read len bytes from socket into buf
	int read;

	if((read = recv(connectedSocket, (char*) buf, len, MSG_WAITALL)) < 0) throw SocketException("Could not read len bytes from descriptor.");
	
	return read; // return number of bytes read
}