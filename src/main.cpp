/* #include "LibAVHLS.h" */

#include "AbstractSocket.h"
#include <opencv2/opencv.hpp>
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

extern "C" {
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libavutil/avutil.h"
	#include "libavutil/opt.h"
	#include "libavutil/log.h"
	#include <libswscale/swscale.h>
}

using namespace cv;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

bool directoryExists(std::string path) {
	struct stat info;
	if(stat(path.c_str(), &info) != 0) return false;
	return (info.st_mode & S_IFDIR);
}

void cameraThreadWorker(std::string directory, int descriptor) {
	std::cout << "\033[0;33mWorker Thread " << std::this_thread::get_id() << " opened.\033[0m"  << std::endl;
	
	AbstractSocket socket = AbstractSocket(descriptor);
	
	char keepAlive[1];
	int len;
	int id;

	AVCodec *codec;
	AVCodecContext *c= NULL;
	int frame, got_picture;

	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
			fprintf(stderr, "codec not found\n");
			exit(1);
	}

	c = avcodec_alloc_context3(codec);

	if((codec->capabilities) & AV_CODEC_CAP_TRUNCATED)
			(c->flags) |= AV_CODEC_FLAG_TRUNCATED;

	c->height = 480;
	c->width = 640;

	uint8_t* data = new uint8_t[c->width * c->height];

	if (avcodec_open2(c, codec, NULL) < 0) {
			fprintf(stderr, "could not open codec\n");
			exit(1);
	}

	try {
		socket.read(&id, sizeof(int));
		std::cout << "\033[0;35m[Worker Thread " << std::this_thread::get_id() << "]\033[0;32m Camera UUID is " << id << "\033[0m" << std::endl;
	} catch(std::exception& e) {
		std::cout << e.what() << std::endl;
		socket.~AbstractSocket();
		return;
	}

	AVPacket avpkt;
	av_init_packet(&avpkt);
	AVFrame *picture;
	picture = av_frame_alloc();

	while(1) {
		try{
			/* socket.write(keepAlive, 1); */
			using std::chrono::high_resolution_clock;
			auto t1 = high_resolution_clock::now();
			socket.read(&len, sizeof(int));
			socket.read(data, len);
			auto t2 = high_resolution_clock::now();
			auto ms_int = duration_cast<milliseconds>(t2 - t1);
			std::cout << ms_int.count() << std::endl;
			avpkt.size = len;
			avpkt.data = data;

			avcodec_decode_video2(c, picture, &got_picture, &avpkt);

			if (got_picture) {
				int w = c->width;
				int h = c->height;
				AVFrame dst;

				Mat mat(h, w, CV_8UC3);
				dst.data[0] = (uint8_t *)mat.data;
				avpicture_fill( (AVPicture *)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);

				enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)picture->format;
				enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;
				struct SwsContext* convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt,
														SWS_FAST_BILINEAR, NULL, NULL, NULL);

				if(convert_ctx == NULL) {
						fprintf(stderr, "Cannot initialize the conversion context!\n");
						exit(1);
				}

				sws_scale(convert_ctx, picture->data, picture->linesize, 0, h,
                    dst.data, dst.linesize);



				imshow("MyVideo", mat);
				waitKey(1);
			}

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
			
			/* std::thread t = std::thread(&cameraThreadWorker, baseDirectory, socket.connectedSocket); */
			cameraThreadWorker(baseDirectory, socket.connectedSocket);
			/* t.detach(); */
		
		} catch(std::exception& e){
			std::cout << e.what() << std::endl;
			return 1;
	
		}
	}
}
