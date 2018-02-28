#include <cstring>
#include <cstdlib>
#include "frame.h"

VncFrame::VncFrame(FrameFormat format)
	:fformat(format)
{
	buffer = std::malloc(fformat.gets());
}


VncFrame::VncFrame(void* source, FrameFormat format)
	: fformat(format)
{
	buffer = std::malloc(fformat.gets());
	std::memcpy(buffer, source, fformat.gets());
}

void VncFrame::flushFrame(void *source){
	if(buffer)
		std::memcpy(buffer, source, fformat.gets());
}
	
void VncFrame::copyFrame(void *dest){
	if(buffer)
		std::memcpy(dest, buffer, fformat.gets());
}

Packet& VncFrame::makePacket(){
	Packet p( fformat.gets(),  buffer);
	return p;
}

VncFrame::~VncFrame(){
	if(buffer)
		std::free(buffer);
}