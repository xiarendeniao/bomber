#pragma once

#include "curl/curl.h"
#include <pthread.h>
#include "Singleton.h"
#include "cocos2d.h"
#include "WorldPacket.h"

enum CONNECT_STATUS
{
	CONNECT_INIT	= 0,	//初始状态
	CONNECT_ING		= 1,	//正在连接
	CONNECT_FAILED	= 2,	//连接失败
	CONNECT_SUCCESS	= 3,	//连接建立
	CONNECT_LOST	= 4		//连接丢失
};

enum Opcodes
{
	MSG_NULL_ACTION						= 0,
	CMSG_PING							= 1,
	SMSG_PONG							= 2,
	CMSG_LOGIN							= 3,
	SMSG_LOGIN							= 4,
	SMSG_LOGOUT							= 5,
	SMSG_RESULT							= 6,
	CMSG_MOVE							= 7,
	SMSG_MOVE							= 8,
	SMSG_ROOM_LIST						= 9,
	CMSG_ROOM_CREATE					= 10,
	CMSG_ROOM_ENTER						= 11,
	SMSG_ROOM_INFO						= 12,
	CMSG_ROOM_READY						= 13,
	CMSG_ROOM_LEAVE						= 14,
	SMSG_ROOM_LEAVE						= 15,
	SMSG_MAP_ENTER						= 16,
	CMSG_BOMB_BORN						= 17,
	SMSG_BOMB_BORN						= 18,
	SMSG_BOMB_EXPLODE					= 19,
	SMSG_ROLE_DEAD						= 20,
	SMSG_GAME_OVER						= 21,

	MMSG_CONNECT_STATUS					= 500,
	MSG_NUM_TYPES
};

struct Location {
	uint16 x;
	uint16 y;
};

//class Connection: public Singleton<Connection>
class Connection
{
public:
	Connection(void);
	~Connection(void);
	void Run();
	bool ConnectToServer(char** errInfo = NULL);
	void SendToServer(WorldPacket* pkg);
	void StopNetwork();

private:
	void ChangeConnectStatus(CONNECT_STATUS status);
	void DEBUG_SIMULATE_SMSG();

public:
	CONNECT_STATUS m_connectStatus;
	bool m_threadToExit;

private:
	CURL *curl;							//libcurl的对象
	curl_socket_t sockfd;				//libcurl打开的socket
	pthread_mutex_t m_mutexForVec;		//操作m_sendPkgVec的线程锁
	pthread_mutex_t m_mutexForSend;		//发送数据的线程锁

	uint8* m_readBuffer;				//接收数据缓冲区，存放还没接收完整的包数据
	uint8* m_writeBuffer;				//发送数据缓冲区，存放正在发送的整包数据
	int m_readCursor;					//接收数据缓冲区已填充长度
	int m_writeCursor;					//发送数据已发送长度
	int m_writeLen;						//需要发送数据的总长度
	volatile bool m_sendDirect;			//fd是否可以直接发送数据

	vector<WorldPacket*> m_sendPkgVec;	//待发送数据包队列

	const uint32 BUFFER_LEN;
	pthread_t m_threadInst;					//pthread的对象
};

class WorldPacket;
class PublicMsgHandler:public cocos2d::CCNode, public Singleton<PublicMsgHandler>
{
private:
	vector<WorldPacket*> m_pkgVec;
	pthread_mutex_t mutex;
	void            WaitMutex() { pthread_mutex_lock(&mutex); }
	void            ClearMutex() { pthread_mutex_unlock(&mutex); }

public:
	PublicMsgHandler();
	~PublicMsgHandler();
	virtual void update(float fDelta);
	void addPacket(WorldPacket* pkg);
};
