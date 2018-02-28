#ifndef VNC_SERVER_H
#define VNC_SERVER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>

#include "grabber.h"
#include "encoder.h"
#include "server.h"
#include "bufferQueue.h"
#include "packetPool.h"

const int BUFFER_NUMBERS = 10;
const int BIT_RATE = 400000;

// daemon 线程状态
typedef enum{
	EXIT_VNC_SERVER = 0,
	VNC_SERVER_RUNABLE = 1
}DaemonThreadStatus;

// 所有work线程状态
typedef enum{
	STOP_BY_DAEMON = 0,
	STOP_OF_LACKING_RESOURCE = 1,
	WORKING_RUNABLE = 2
}WorkThreadStatus;

// encoder处于可运行形式下的状态
typedef enum{
	ENCODER_RUNNING = 0,
	ENCODER_SUSPAND_LACKING_FRAME = 1,
	ENCODER_SUSPAND_LACKING_PACKET = 2
}EncoderStatus;

// netServer线程处于可运行态下的状态
typedef enum{
	NET_SERVER_RUNNING = 0,
	NET_SERVER_LACKING_PACKET = 1
}NetServerStatus;

class VncServer{
public:
	VncServer(int nums = BUFFER_NUMBERS, int encodeBitRate = BIT_RATE);

	virtual ~VncServer();

	void Grabber_thread();

	void NetServer_thread();

	void Encoder_thread();

	void Daemon_thread();

	void setDaemonThreadStatus(DaemonThreadStatus status);

private:
	bool exitDaemonThread();

	void setWorkThreadStatus(WorkThreadStatus status);
	bool stopWorkThread();

	void setEncoderThreadStatus(EncoderStatus status);
	bool matchEncoderThreadStatus(EncoderStatus status);

	void setNetServerThreadStatus(NetServerStatus status);
	bool matchNetServerThreadStatus(NetServerStatus status);

public:
	void netServer_establishConnect();
	void netServer_lossConnect();

private:

	// 通过网络连接的建立、断开来控制daemon线程
	std::mutex connect_mutex;
	std::condition_variable establish_connect;
	std::condition_variable loss_connect;
	bool netNotified;

	// daemon控制work线程
	std::mutex work_mutex;
	std::condition_variable start_work;
	std::condition_variable stop_work;

	// daemon与work线程的通知控制
	bool daemonNotify_Grabber;
	bool grabberNotify_Daemon;

	bool daemonNotify_Encoder;
	bool encoderNotify_Daemon;

	bool daemonNotify_NetServer;
	bool netServerNotify_Daemon;

	// encoder线程状态(处于可运行状态下)
	// 仅被自身设置
	EncoderStatus encoder_status;

	// netServer线程状态
	// 仅可被自身设置
	NetServerStatus netServer_status;

	// grabber与 netServer 唤醒encoder
	std::mutex encode_mutex;
	std::condition_variable wake_encode;
	bool grabberWakeEncoder;
	bool netServerWakeEncoder;

	// encoder 唤醒 netServer
	std::mutex netServer_mutex;
	std::condition_variable wake_netServer;
	bool encoderWakeNetServer;

	std::shared_ptr<Grabber> grabber;

	std::shared_ptr<Encoder> encoder;
	int bitrate;

	std::shared_ptr<BufferQueue> bufferQueue;
	int numsOfBuffer;

	std::shared_ptr<PacketPool> pool;
	int sizeOfPool;

	std::shared_ptr<NetServer> server;

	FrameFormat format;
	FrameFormat out_format;

	std::thread m_GrabberThread;
	std::thread m_NetServerThread;
	std::thread m_EncoderThread;
	std::thread m_DaemonThread;

	// 仅被daemon进程设置，work线程读
	// 不需要互斥锁
	DaemonThreadStatus dStatus;
	WorkThreadStatus wStatus;
};

#endif