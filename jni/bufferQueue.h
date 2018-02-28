#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <list>
#include <mutex>
#include "frame.h"

class BufferQueue
{
public:
	BufferQueue(){}
	BufferQueue(int size, FrameFormat format);
	virtual ~BufferQueue();

	VncFrame* popNewFrame();

	void pushNewFrame(void* source);

	void pushUsedFrame(VncFrame* f);

private:
	void clearFreshQueue();

private:
	// 维护从系统获取到的frame
	std::list<VncFrame*> fresh;

	// 维护已被使用过的frame
	// 等待被再次使用
	std::list<VncFrame*> used;
	// 允许连续丢帧数
	// 超过阈值，则清空现有fresh队列
	int tolerance;
};

#endif
