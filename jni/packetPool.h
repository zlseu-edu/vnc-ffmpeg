#ifndef PACKET_POOL_H
#define PACKET_POOL_H

#include <list>
#include <mutex>

class Packet{
public:
	Packet(int lhs, void *rhs)
		:size(lhs), data(rhs){}
	
	int size;
	void* data;
};

class PacketPool{
public:
	PacketPool(int s);

	virtual ~PacketPool();

	bool pushNewPacket(const Packet &packet);
	Packet& popNewPacket();
	void pushUsedPacket(const Packet &packet);

private:
	std::list<Packet> cached;

	// 初始情况下有1个成员
	// 一般情况下将有两个成员
	std::list<Packet> usable;

	int size;
	void *pool;

	std::mutex lock;
};

#endif