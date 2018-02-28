#include <cstring>
#include <cstdlib>
#include <iterator>
#include "packetPool.h"

PacketPool::PacketPool(int s): size(s){
	pool = malloc(size);

	usable.push_back(Packet(size, pool));
}

PacketPool::~PacketPool(){
	if(pool)
		free(pool);
}

bool PacketPool::pushNewPacket(const Packet &packet){
	if(usable.empty())
		return false;

	std::lock_guard<std::mutex> lck(lock);

	std::list<Packet>::iterator iter = usable.begin();
	//  首先检测后一个成员是否可写入
	if(usable.rbegin()->size >= packet.size)
		 iter = (++iter == usable.end() ? usable.begin() : iter);
	else if(usable.begin()->size >= packet.size)
		iter = usable.begin();
	else
		return false;

	Packet p(packet.size, iter->data);
	std::memcpy(p.data, packet.data, packet.size);

	// 挂入缓存队列
	cached.push_back(p);
	// 切割该packet
	if(iter->size == packet.size)
		usable.erase(iter);
	else{
		// 调整packet
		iter->data = (void*)((char *)iter->data + packet.size);
		iter->size -= packet.size;
	}

	return true;
}

Packet& PacketPool::popNewPacket(){
	Packet packet(0, nullptr);
	if(cached.empty())
		return packet;
	packet = *cached.begin();
	cached.pop_front();
	return packet;
}

void PacketPool::pushUsedPacket(const Packet &packet){
	std::lock_guard<std::mutex> lck(lock);

	std::list<Packet>::iterator pos = usable.begin();
	if((char*)pos->data + pos->size != (char*)packet.data)
		usable.push_front(packet);
	else
		pos->size += packet.size;
	
	// 检测刚刚聚合的第一块内存块与第二块是否可以再次聚合
	if( (char*)packet.data + packet.size == (char*)usable.rbegin()->data ){
		pos->size += usable.rbegin()->size; 
		usable.pop_back();
	}
	return;
}