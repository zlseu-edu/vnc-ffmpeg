#include <iterator>
#include "bufferQueue.h"

BufferQueue::BufferQueue(int size, FrameFormat format){
	for(int i = 0; i < size; ++i)
		used.push_back(new VncFrame(format));

	tolerance = size / 2;
}

BufferQueue::~BufferQueue(){

	for(std::list<VncFrame*>::iterator iter = fresh.begin(); iter != fresh.end(); ++iter)
		delete *iter;

	for(std::list<VncFrame*>::iterator iter = used.begin(); iter != used.end(); ++iter)
		delete *iter;
}

VncFrame* BufferQueue::popNewFrame(){
	// 无新frame
	if(fresh.empty())
		return nullptr;

	VncFrame* head = *(fresh.begin());
	fresh.pop_front();
	return head;
}

void BufferQueue::pushNewFrame(void* source){

	static int current_loss = 0;

	// 无可用旧frame
	if(used.empty()){
		++current_loss;
		return;
	}

	current_loss = 0;

	VncFrame* head = *(used.begin());
	used.pop_front();
	
	head->flushFrame(source);

	fresh.push_back(head);
	return;
}

void BufferQueue::pushUsedFrame(VncFrame* f){
	used.push_back(f);
	f = nullptr;
}