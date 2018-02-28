#ifndef ENCODER_H
#define ENCODER_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

#include "frame.h"

#define DEFAULT_AUV_FORMAT AV_PIX_FMT_YUV420P

#define CACULATE_INPUT_PORMAT(format) \
	format == 4 ? AV_PIX_FMT_RGB32: AV_PIX_FMT_RGB565

class Encoder
{
private:
	FrameFormat in;
	FrameFormat out;

	AVCodec *codec;
	AVCodecContext *context;
	AVFrame *frame;

	AVPacket packet;
	SwsContext *sws;

private:
	AVPixelFormat transformat(int f);

public:
	Encoder(FrameFormat input, FrameFormat output, int bitrate);

	void encode(VncFrame *source, Packet *dest);

	~Encoder();
};

#endif