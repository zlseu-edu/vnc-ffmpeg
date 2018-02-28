#include <iostream>
#include <cstdlib>

#include "encoder.h"

Encoder::Encoder(FrameFormat input, FrameFormat output, int bitrate)
	: out(output) , in(input) 
{
	int ret;

	avcodec_register_all();

	/* find the mpeg video encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
	if (!codec) {
        std::cerr << "Codec not found." << std::endl;
        exit(1);
    }

	context = avcodec_alloc_context3(codec);
	if (!context) {
       	std::cerr << "Could not allocate video codec context." << std::endl;
    	exit(1);
    }

	context->dct_algo = FF_DCT_FASTINT;

	/* set sample parameters */
	context->bit_rate = bitrate;

	/* resolution must be a multiple of two */
	context->width = out.getw();
	context->height = out.geth();

	/* frame per second */
	context->time_base.num = 1;
	context->time_base.den = 30;

	/*  emit one intra frame every 30 frames */
	context->gop_size = 30;
	context->max_b_frames = 0;
	context->pix_fmt = DEFAULT_AUV_FORMAT;

	if(avcodec_open2(context, codec, NULL)){
		std::cerr << "Could not open codec." << std::endl;
        exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
         std::cerr << "Could not allocate video frame." << std::endl;
         exit(1);
     }

	frame->format = DEFAULT_AUV_FORMAT;
	frame->width  = out.getw();
	frame->height = out.geth();
	frame->pts = 0;

	ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		std::cout << "Could not allocate the video frame data." << std::endl;
		exit(1);
	}

	sws = sws_getContext(
		in.getw(), in.geth(), CACULATE_INPUT_PORMAT(in.getf()),
		out.getw(), out.geth(), DEFAULT_AUV_FORMAT,
		SWS_FAST_BILINEAR, 0, 0, 0
	);
	if(!sws){
		std::cerr << "Could not allocate SwsContext." << std::endl;
    	exit(1);
	}
}

Encoder::~Encoder() {
	if(sws)
		sws_freeContext(sws);

	if(context)
		avcodec_free_context(&context);
	
	if(frame)
		av_frame_free(&frame);
}

void Encoder::encode(VncFrame *source, Packet *dest) {

	uint8_t *in_data[1] = {(uint8_t *)( (source->makePacket()).data )};
	int in_linesize[1] = { in.get_stride() };
	sws_scale(sws, in_data, in_linesize, 0, in.geth(), frame->data, frame->linesize);
		
	int available_size = dest->size;
	dest->size = 0;
	frame->pts++;
	
	av_init_packet(&packet);
	int success = 0;
	avcodec_encode_video2(context, &packet, frame, &success);
	if( success ) {
		if( packet.size <= available_size ) {
			memcpy(dest->data, packet.data, packet.size);
			dest->size = packet.size;
		}
		else 
			std::cerr << "Frame too large for buffer (size: "<< available_size << "needed:" << packet.size <<")" << std::endl;
	}
	av_packet_unref(&packet);
}

AVPixelFormat Encoder::transformat(int f){
	AVPixelFormat avPixelFormat;	
	if(f == 4)
		avPixelFormat = AV_PIX_FMT_RGB32;
	else if(f == 2)
		avPixelFormat =  AV_PIX_FMT_RGB565;
	else{
		std::cerr << "Unsupport piexel format, exit(1)."<< std::endl;
		exit(1);
	}
	return avPixelFormat;
}