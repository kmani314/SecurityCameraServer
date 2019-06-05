#include "LibAVHLS.h"
#include "AbstractSocket.h"
#include <vector>
#include <iostream>
#include <thread>

int main() {
	std::vector<unsigned char> data;
	int len;
	AbstractSocket socket;
	LibAVHLS* hls;

	try {
		hls = new LibAVHLS("../template/test.264", "./test.m3u8");
		socket = AbstractSocket();
		socket.listen(8080, 10);
		socket.waitForConnection();

	} catch(std::exception& e){
		std::cout << e.what() << std::endl;
		return 1;
	}

	std::cout << "New Client Connection" << std::endl;

	while(1) {
		try{
			socket.read(&len, sizeof(int));
			data.resize(len);
			socket.read(&data[0], len);
			hls->writeHLSSegment(&data[0], len, 70);
		} catch(std::exception& e) {
			std::cout << e.what() << std::endl;
			return 1;
		}
	}
}
