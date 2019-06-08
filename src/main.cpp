#include "LibAVHLS.h"
#include "AbstractSocket.h"
#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool directoryExists(std::string path) {
	struct stat info;
	if(stat(path.c_str(), &info) != 0) return false;
	return (info.st_mode & S_IFDIR);
}

void cameraThreadWorker(std::string directory, int descriptor) {
	std::cout << "\033[0;33mWorker Thread " << std::this_thread::get_id() << " opened.\033[0m"  << std::endl;
	
	AbstractSocket socket = AbstractSocket(descriptor);
	
	std::vector<unsigned char> data;
	char keepAlive[1];
	int len;
	
	/* Directory Logic*/
	
	LibAVHLS *hls;
	int id;

	try {
		socket.read(&id, sizeof(int));
		std::cout << "\033[0;35m[Worker Thread " << std::this_thread::get_id() << "]\033[0;32m Camera UUID is " << id << "\033[0m" << std::endl;
		directory += "/" + std::to_string(id);
		if(directoryExists(directory)) {
			std::cout << "\033[0;35m[Worker Thread " << std::this_thread::get_id() << "]\033[0;32m Using previously created directory for camera with UUID " << id << " (" << directory << ")" << std::endl;
		} else {
			mkdir(directory.c_str(), 0775);
			std::cout << "\033[0;35m[Worker Thread " << std::this_thread::get_id() << "]\033[0;32m Creating a directory for new camera with UUID " << id << " (" << directory << ")" << std::endl; 
		}
		directory += "/index.m3u8";
		
		hls = new LibAVHLS("../template/test.264", directory.c_str());
		hls->setHLSOption("hls_time", 5, AV_OPT_SEARCH_CHILDREN);
		hls->setHLSOption("use_localtime", 1, 0);
		hls->setHLSOption("strftime", 1, 0);

		hls->writeHLSHeader();

	} catch(std::exception& e) {
		std::cout << e.what() << std::endl;
		socket.~AbstractSocket();
		return;
	}

	
	while(1) {
		try{
			socket.write(keepAlive, 1);
			socket.read(&len, sizeof(int));
			data.resize(len);
			socket.read(&data[0], len);
			hls->writeHLSSegment(&data[0], len, 70);
		} catch(std::exception& e) {
			std::cout << e.what() << std::endl;
			socket.~AbstractSocket();
			break;
		}
	}
	std::cout << "\033[0;35m[Worker Thread " << std::this_thread::get_id() << "]\033[0;32m Camera with UUID " << id << " has disconnected." << " Thread is terminating... \033[0m" << std::endl;
}

int main(int argc, char** argv) {
	struct sigaction action = {SIG_IGN};
	sigaction(SIGPIPE, &action, NULL);

	if(argc < 4) {
		std::cout << "\033[1;31mIncorrect Arguments Supplied.\033[0m\n1. Port to start server on \n2. Maximum number of concurrent connections \n3. Base directory for camera stream directories" << std::endl;
		return 1;
	}

	int port = atoi(argv[1]);
	int connectionLimit = atoi(argv[2]);
	
	std::string baseDirectory(argv[3]);
	if(!directoryExists(baseDirectory)) {
		std::cout << "Supplied Path does not exist or is not a valid directory" << std::endl;
		return 1;
	}
	
	AbstractSocket socket = AbstractSocket();
	socket.listen(port, connectionLimit);
	
	while(1) { // Accept loop
		try {
			socket.waitForConnection();
				
			std::cout << "New Client Connection" << std::endl;
			
			std::thread t = std::thread(&cameraThreadWorker, baseDirectory, socket.connectedSocket);
			t.detach();
		
		} catch(std::exception& e){
			std::cout << e.what() << std::endl;
			return 1;
	
		}
	}
}
