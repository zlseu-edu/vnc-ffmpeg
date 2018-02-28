#ifndef GRABBER_H
#define GRABBER_H

#include "frame.h"

#define ARR_LEN(a) (sizeof(a)/sizeof(a)[0])
static int compiled_sdks[] = {24};

typedef int (*init_flinger)(void);
typedef void (*close_flinger)(void);
typedef void* (*lockBuffer_flinger)(void);
typedef void (*unlockBuffer_flinger)(void);
typedef FrameFormat& (*getFrameBuffer_flinger)(void);

typedef struct flinger{

	int (*init_flinger)();

	void (*close_flinger)();

	void* (*lockBuffer_flinger)();

	void (*unlockBuffer_flinger)();
	
	FrameFormat& (*getFrameFormat_flinger)();
}frame_flinger_t;

class Grabber
{
public:
	Grabber();
	virtual ~Grabber();

	void* lock();
	void unlock();

	FrameFormat& getFrameFormat();
private:
	frame_flinger_t base;
};

#endif