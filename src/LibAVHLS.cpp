#include "LibAVHLS.h"
#include <iostream>
#include <exception>

class LibAVHLSException : public std::exception {
	private:
	const char* msg;
	public:
	LibAVHLSException(const char* _msg) : msg(_msg) {}	
	const char* what() const noexcept {
		return msg;
	}
};

LibAVHLS::~LibAVHLS() {
	av_write_trailer(HLSOutputContext);
	avformat_free_context(parameterTemplateContext);
	avformat_free_context(HLSOutputContext);
}

LibAVHLS::LibAVHLS(const char* templateFile, const char* outFile) {
	int ret;
//	av_log_set_level(AV_LOG_ERROR);

	ret = avformat_open_input(&parameterTemplateContext, templateFile, NULL, NULL);
	if(ret < 0) throw LibAVHLSException("Could not open template file");

	ret = avformat_find_stream_info(parameterTemplateContext, NULL);
	if(ret < 0) throw LibAVHLSException("Failed to read video parameters from template file");

	avformat_alloc_output_context2(&HLSOutputContext, NULL, NULL, outFile);
	if(!HLSOutputContext) throw LibAVHLSException("Could not create an output context for specified file");

	for(int i = 0; i < parameterTemplateContext->nb_streams; i++) {
		AVStream* outStream;
		if(parameterTemplateContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			std::cout << "Found video stream at index: " << i << std::endl;
			streamIndex = i;

			outStream = avformat_new_stream(HLSOutputContext, NULL);

			if(!outStream) throw LibAVHLSException("Could not create an output stream");
			ret = avcodec_parameters_copy(outStream->codecpar, parameterTemplateContext->streams[i]->codecpar);
			if(ret < 0) throw LibAVHLSException("Could not copy input codec parameters");	
			break;
		}
		if(i ==  parameterTemplateContext->nb_streams) throw LibAVHLSException("No video stream found in input");
	}
	
	av_dump_format(HLSOutputContext, 0, outFile, 1);

}

void LibAVHLS::writeHLSHeader() {
	int ret = avformat_write_header(HLSOutputContext, NULL);
	if(ret < 0) throw LibAVHLSException("Could not write file header");

}

void LibAVHLS::writeHLSSegment(unsigned char* data, int len, int msSinceLastFrame) {
	/*
	pts += Timebase / fps
	fps = 1000ms/s / X milliseconds per frame
	pts += Timebase*ms/f / 1000ms/s
	*/

	int ptsDifference = (HLSOutputContext->streams[streamIndex]->time_base.den*msSinceLastFrame) / 1000;
	currentPTS += ptsDifference;

	AVPacket packet; 
	av_init_packet(&packet);
		
	packet.flags |= AV_PKT_FLAG_KEY;
	packet.stream_index = streamIndex;
	packet.data = data;
	packet.size = len;
	packet.duration = ptsDifference;
	packet.pts = packet.dts = currentPTS;

	if(av_interleaved_write_frame(HLSOutputContext, &packet) < 0) throw LibAVHLSException("Could not write frame");
	av_packet_unref(&packet);
}

void LibAVHLS::setHLSOption(const char* key, int value, int search) {
	int ret = av_opt_set_int(HLSOutputContext->priv_data, key, value, search);
	if(ret < 0) throw LibAVHLSException("Could not set option");
}

void LibAVHLS::setHLSOption(const char* key, const char* value, int search) {
	int ret = av_opt_set(HLSOutputContext->priv_data, key, value, search);
	if(ret < 0) throw LibAVHLSException("Could not set option");
}
