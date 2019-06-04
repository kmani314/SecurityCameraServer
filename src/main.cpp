extern "C" {
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libavutil/opt.h"
}

#include <iostream>
#include <AbstractSocket.h>
#include <vector>

struct cameraHeader {
	int cameraID;
	int frameWidth;
	int frameHeight;
};

int main() {
	int ret;
	int pos;
	int len;
	std::vector<unsigned char> data;

	const char* outFile = "/Users/camera-server/video/index.m3u8";
	const char* templateFile = "../template/test.264";

	struct AVFormatContext* inputContext = NULL;
	
	ret = avformat_open_input(&inputContext, templateFile, NULL, NULL);

	if(ret < 0) {
		std::cout << "could not open template file" << std::endl;
		return 1;
	}
	
	ret = avformat_find_stream_info(inputContext, NULL);

	if(ret < 0) {
		std::cout << "Could not find stream info" << std::endl;
		return 1;
	}

	struct AVFormatContext* outputContext = NULL;
	avformat_alloc_output_context2(&outputContext, NULL, NULL, outFile);

	if(!outputContext) {
		std::cout << "could not create output context" << std::endl;
		return 1;
	}

	int numStreams = inputContext->nb_streams;
	int streamIndex = 0;

	for(int i = 0; i < numStreams; i++) {
		AVStream* outStream;
		AVStream* inStream = inputContext->streams[i];
		AVCodecParameters* params = inStream->codecpar;

		if(params->codec_type == AVMEDIA_TYPE_VIDEO) {
			std::cout << "found video stream";
			streamIndex = i;
			outStream = avformat_new_stream(outputContext, NULL);
			if(!outStream) {
				std::cout << "could not create outputstream" << std::endl;
				return 1;
			}
			ret = avcodec_parameters_copy(outStream->codecpar, params);
			if(ret < 0) {
				std::cout << "could not copy codec params!" << std::endl;
				return 1;
			}
			break;
		}
		if(i == numStreams) {
			std::cout << "No video stream found on input to copy parameters" << std::endl;
			return 1;
		}
	}
	av_dump_format(outputContext, 0, outFile, 1);
	
	av_opt_set_int(outputContext->priv_data, "hls_time", 10, 0);
	av_opt_set_int(outputContext->priv_data, "start_number", 0, 0);
	
	ret = avio_open(&outputContext->pb, outFile, AVIO_FLAG_WRITE);
	
	if(ret < 0) {
		std::cout << "Fatal: Could not open \"" << outFile << "\" for writing" << std::endl;
		return 1;
	}
	
	ret = avformat_write_header(outputContext, NULL);
	
	if(ret < 0) {
		std::cout << "Fatal: Could not write file header" << std::endl;
		return 1;
	}

	
	int64_t frameTime;
	int64_t frameDuration;
	
	int64_t count = 0;
	
	AbstractSocket socket = AbstractSocket();
	socket.listen(8080, 10);
	socket.waitForConnection();
	std::cout << "New Connection!" << std::endl;
	/*std::cout << "---- Camera Info --- " << std::endl;
	
	struct cameraHeader cameraInfo;
	memset(&cameraInfo, 0, sizeof(cameraInfo));

	socket.read(&cameraInfo.cameraID, sizeof(int));
	std::cout << "ID: " << cameraInfo.cameraID << std::endl;
	
	socket.read(&cameraInfo.frameWidth, sizeof(int));
	socket.read(&cameraInfo.frameHeight, sizeof(int));
	std::cout << "Resolution: " << cameraInfo.frameWidth << "x" << cameraInfo.frameHeight << std::endl;
*/	
	while(1) {
		AVStream* outputStream = outputContext->streams[streamIndex];
		
		socket.read(&len, 4);
		data.resize(len);
		socket.read(&data[0], len);
		frameDuration = outputStream->time_base.den / 15;
		frameTime = count*frameDuration;

		AVPacket packet;
        av_init_packet(&packet);
		
		packet.flags |= AV_PKT_FLAG_KEY;

		packet.stream_index = streamIndex;		
		packet.data = &data[0];
		packet.size = len;
		packet.duration = frameDuration;
		packet.dts = packet.pts = frameTime / (int64_t) outputStream->time_base.num;
		
		ret = av_interleaved_write_frame(outputContext, &packet);
		if(ret < 0) {
			std::cout << "Failed to write frame" << std::endl;
			return 1;
		}
		
		count++;
		av_packet_unref(&packet);
	}
	av_write_trailer(outputContext);

	avformat_free_context(outputContext);
}
