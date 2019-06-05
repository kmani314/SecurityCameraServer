#ifndef LIBAVHLS_H
#define LIBAVHLS_H
extern "C" {
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libavutil/opt.h"
}

#include <iostream>

class LibAVHLS {
	private:
	AVFormatContext* parameterTemplateContext = NULL;
	AVFormatContext* HLSOutputContext = NULL;

	int64_t currentPTS = 0;
	
	AVRational h264Timebase;
	int streamIndex;
	
	public:
	LibAVHLS(const char*, const char*);
	~LibAVHLS(); 
	
	void writeHLSSegment(unsigned char*, int, int); 
};
#endif