#include "vncServer.h"

VncServer::VncServer(int nums, int encodeBitRate)
	: numsOfBuffer(nums) 
	, bitrate(encodeBitRate)
	, grabber(new Grabber())
	, server(new NetServer("/data/mpeg-test.mpeg"))
{
	daemonNotify_Grabber = false;
	grabberNotify_Daemon = false;
	daemonNotify_Encoder = false;
	encoderNotify_Daemon = false;
	daemonNotify_NetServer = false;
	netServerNotify_Daemon = false;
	grabberWakeEncoder = false;
	netServerWakeEncoder = false;
	encoderWakeNetServer = false;
	netNotified = false;
	bufferQueue = nullptr;
	pool = nullptr;

	// 帧格式
	format = grabber->getFrameFormat();

	// Pool大小设为一帧的大小
	sizeOfPool = format.gets();

	setDaemonThreadStatus(VNC_SERVER_RUNABLE);

	m_GrabberThread = std::thread( std::bind(&VncServer::Grabber_thread, this) );	
	m_NetServerThread = std::thread( std::bind(&VncServer::NetServer_thread, this) );
	m_EncoderThread = std::thread( std::bind(&VncServer::Encoder_thread, this) );
	m_DaemonThread = std::thread( std::bind(&VncServer::Daemon_thread, this) );
}

VncServer::~VncServer(){
	m_GrabberThread.join();
	m_NetServerThread.join();
	m_EncoderThread.join();
	m_DaemonThread.join();
}

void VncServer::setDaemonThreadStatus(DaemonThreadStatus status){
	dStatus = status;

	if(dStatus == EXIT_VNC_SERVER){
		// 清理现场
	}
}

bool VncServer::exitDaemonThread(){
	return dStatus == EXIT_VNC_SERVER;
}

void VncServer::setWorkThreadStatus(WorkThreadStatus status){
	wStatus = status;
}

bool VncServer::stopWorkThread(){
	return wStatus != WORKING_RUNABLE; 
}

void VncServer::setEncoderThreadStatus(EncoderStatus status){
	encoder_status = status;
}
	
bool VncServer::matchEncoderThreadStatus(EncoderStatus status){
	return encoder_status == status;
}

void VncServer::setNetServerThreadStatus(NetServerStatus status){
	netServer_status = status;
}

bool VncServer::matchNetServerThreadStatus(NetServerStatus status){
	return netServer_status == status;
}

void VncServer::netServer_establishConnect(){
	netNotified = true;
	establish_connect.notify_all();
}

void VncServer::netServer_lossConnect(){
	netNotified = true;
	loss_connect.notify_all();
}

void VncServer::Grabber_thread(){

	printf(" Grabber_thread start... ");
	while(true){
		// 等待daemon线程开启本线程
		if( stopWorkThread() ){
			std::unique_lock<std::mutex> work_lock(work_mutex);
			while( !daemonNotify_Grabber )
				start_work.wait(work_lock);
			daemonNotify_Grabber = false;
		}

		void *buffer;

		while(true){
			// 本线程通知daemon线程已准备好停止操作
			if( stopWorkThread() ){
				grabberNotify_Daemon = true;
				stop_work.notify_one();
				break;
			}

			buffer = grabber->lock();
			bufferQueue->pushNewFrame(buffer);
			grabber->unlock();

			if( matchEncoderThreadStatus( ENCODER_SUSPAND_LACKING_FRAME ) ){
				grabberWakeEncoder = true;
				wake_encode.notify_one();
			}
		}
		printf(" Grabber_thread stopped by daemon. ");
	}
}

void VncServer::NetServer_thread(){
	printf(" NetServer_thread start... ");
	while(true){
		
		// 等待daemon线程开启本线程
		if( stopWorkThread() ){
			std::unique_lock<std::mutex> work_lock(work_mutex);
			while( !daemonNotify_NetServer )
				start_work.wait(work_lock);
			daemonNotify_NetServer = false;
		}

		Packet packet(0, nullptr);

		while(true){

			// 本线程通知daemon线程已准备好停止操作
			if( stopWorkThread() ){
				netServerNotify_Daemon = true;
				stop_work.notify_one();
				break;
			}

			packet = pool->popNewPacket();

			// 休眠
			if( packet.data == nullptr){
				setNetServerThreadStatus(NET_SERVER_LACKING_PACKET);
				std::unique_lock<std::mutex> netServer_lock(netServer_mutex);
				while( encoderWakeNetServer )
					wake_netServer.wait(netServer_lock);
				encoderWakeNetServer = false;

				// 重新取
				packet = pool->popNewPacket();
			}

			// 检测encoder线程是否因为缺少packet而被阻塞
			if( matchEncoderThreadStatus(ENCODER_SUSPAND_LACKING_PACKET) ){
				netServerWakeEncoder = true;
				wake_encode.notify_one();
			}

			server->cache_one_packet(packet);
		}

		printf(" NetServer_thread stopped by daemon. ");
	}
}

void VncServer::Encoder_thread(){

	printf(" Encoder_thread start... ");
	while(true){

		// 等待daemon线程开启本线程
		if( stopWorkThread() ){
			std::unique_lock<std::mutex> work_lock(work_mutex);
			while( !daemonNotify_Encoder )
				start_work.wait(work_lock);
			daemonNotify_Encoder = false;
		}

		setEncoderThreadStatus(ENCODER_RUNNING);

		VncFrame* vf = nullptr; 

		while(true){
			// 本线程通知daemon线程已准备好停止操作
			if( stopWorkThread() ){
				encoderNotify_Daemon = true;
				stop_work.notify_one();
				break;
			}

			vf = bufferQueue->popNewFrame();

			// 休眠
			if(vf == nullptr){
				setEncoderThreadStatus( ENCODER_SUSPAND_LACKING_FRAME );
				std::unique_lock<std::mutex> encode_lock(encode_mutex);
				while( !grabberWakeEncoder )
					wake_encode.wait(encode_lock);
				grabberWakeEncoder = false;
				setEncoderThreadStatus( ENCODER_RUNNING );

				// 重新去取帧
				vf = bufferQueue->popNewFrame();;
			}

			Packet temp = vf->makePacket();
			encoder->encode(vf, &temp);

			if( pool->pushNewPacket(temp) == false ){
				setEncoderThreadStatus( ENCODER_SUSPAND_LACKING_PACKET );
				std::unique_lock<std::mutex> encode_lock(encode_mutex);
				while( !netServerWakeEncoder )
					wake_encode.wait(encode_lock);
				netServerWakeEncoder = false;
				setEncoderThreadStatus( ENCODER_RUNNING );

				// 重新push
				pool->pushNewPacket(temp);
			}

			// 检测netServer是否因为缺少packet而被阻塞
			if( matchNetServerThreadStatus(NET_SERVER_LACKING_PACKET) ){
				encoderWakeNetServer = true;
				wake_netServer.notify_one();
			}

			// 将frame帧还回队列
			bufferQueue->pushUsedFrame(vf);
			vf = nullptr;
		}

		if(vf)
			delete vf;

		printf(" Encoder_thread stopped by daemon. ");
	}
}

void VncServer::Daemon_thread(){

	printf(" Daemon_thread start... ");

	while(true){

		setWorkThreadStatus(STOP_OF_LACKING_RESOURCE);

		std::unique_lock<std::mutex> connect_lock(connect_mutex);

		// 阻塞等待网络连接建立
		while( !netNotified )
			establish_connect.wait(connect_lock);
		netNotified = false;

		printf(" Got notification from NetServer, starting create resource... ");
		// 创建、申请资源
		encoder = std::shared_ptr<Encoder>(new Encoder(format, out_format, bitrate));
		bufferQueue = std::shared_ptr<BufferQueue>(new BufferQueue(numsOfBuffer, format));
		pool = std::shared_ptr<PacketPool>(new PacketPool(sizeOfPool));

		// 唤醒 grabber、encoder线程
		daemonNotify_Grabber = true;
		daemonNotify_Encoder = true;
		daemonNotify_NetServer = true;
		setWorkThreadStatus(WORKING_RUNABLE);
		start_work.notify_all();

		// 阻塞等待网络断开
		while( !netNotified )
			loss_connect.wait(connect_lock);
		netNotified = false;

		printf(" NetServer loss any connection, starting destroy resource... ");
		
		setWorkThreadStatus(STOP_BY_DAEMON);

		{
			std::unique_lock<std::mutex> work_lock(work_mutex);
			while( !grabberNotify_Daemon )
				stop_work.wait(work_lock);
			grabberNotify_Daemon = false;

			while( !netServerNotify_Daemon )
				stop_work.wait(work_lock);
			netServerNotify_Daemon = false;

			while( !encoderNotify_Daemon )
				stop_work.wait(work_lock);
			encoderNotify_Daemon = false;
		}

		// 释放资源
		encoder = nullptr;
		bufferQueue = nullptr;
		pool = nullptr;

		// 手动释放锁
		connect_lock.unlock();
	}
}