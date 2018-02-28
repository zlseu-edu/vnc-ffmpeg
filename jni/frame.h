#ifndef FRAME_H
#define FRAME_H

#include "packetPool.h"

class FrameFormat
{
private:
	int width;
	int height;
	int format;

	int stride;
	
public:
	FrameFormat(int w = 0, int h = 0, int f = 0)
	: width(w)
	, height(h)
	, format(f)
	{ 
		stride = width * format;
	}

	int getw(){ return width; }
	int geth(){ return height; }
	int getf(){ return format; }

	int get_stride(){ return stride; }
	int gets(){ return stride*height; }
};

class VncFrame
{
public:
	VncFrame(FrameFormat format);
	VncFrame(void* source, FrameFormat format);

	void flushFrame(void *source);
	void copyFrame(void *dest);

	bool empty(){ return buffer == nullptr; }

	void* lock(){ return buffer; }
	void unlock(){}

	Packet& makePacket();
	
	virtual ~VncFrame();

private:
	FrameFormat fformat;
	void* buffer; 
};
#endif