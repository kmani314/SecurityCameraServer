#include "Socket.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>
// #include <opencv/cv.hpp>
// #include <opencv2/imgcodecs/imgcodecs.hpp>
// using namespace cv;
int main()
{
    cv::Mat image = cv::imread("error.png", 0);// read the file
    cv::namedWindow( "Display window");// create a window for display.
    cv::imshow( "Display window", image );// show our image inside it.
    cv::waitKey(0);// wait for a keystroke in the window
    return 0;
}

#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

void cameraThreadWorker(int descriptor) {
	Socket socket = Socket(descriptor);
	int len;
	int frame, got_picture;
	int height = 480;
	int width = 640;

	uint8_t* data = new uint8_t[width * height];

	while(1) {
		try {
			socket.read(&len, sizeof(int));
			socket.read(data, len);

			int w = 640;
			int h = 480;

			// cv::Mat image = cv::Mat::zeros(300, 600, CV_8UC3);
			cv::Mat image;

			// mat.data = data;

			// imshow("robot", mat);
			// waitKey(1);
		} catch(std::exception& e) {
			std::cout << e.what() << std::endl;
			socket.~Socket();
			break;
		}
	}
}

// int main(int argc, char** argv) {
// 	// struct sigaction action = {SIG_IGN};
// 	// sigaction(SIGPIPE, &action, NULL);

// 	int port = atoi(argv[0]);

// 	Socket socket = Socket();
// 	socket.listen(port, 1);
	
// 	while(1) {
// 		try {
// 			socket.waitForConnection();
// 			std::cout << "New Client Connection" << std::endl;
// 			cameraThreadWorker(socket.connectedSocket);
// 		} catch(std::exception& e){
// 			std::cout << e.what() << std::endl;
// 			return 1;
// 		}
// 	}
// }