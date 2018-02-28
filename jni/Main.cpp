#include <iostream>
#include <chrono>
#include <thread>
#include "vncServer.h"

int main(){
	// 初始化截屏方法
	std::cout << "main thread create vncServer." <<std::endl;
	std::shared_ptr<VncServer>  vncServer( new VncServer() );

	std::cout << " main thread sleep 5s... "<< std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	// 工作
	vncServer->netServer_establishConnect();
	std::cout << " main thread wake vncServer start work. " << std::endl;
	
	std::this_thread::sleep_for(std::chrono::milliseconds(200000));

	std::cout << " After sleep 200s, main thread stop work." << std::endl;
	vncServer->netServer_lossConnect();

	std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	return 0;
}