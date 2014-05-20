#pragma once

#include "curl/curl.h"
#include <pthread.h>
#include "Singleton.h"
#include "cocos2d.h"
#include "WorldPacket.h"

enum CONNECT_STATUS
{
	CONNECT_INIT	= 0,	//��ʼ״̬
	CONNECT_ING		= 1,	//��������
	CONNECT_FAILED	= 2,	//����ʧ��
	CONNECT_SUCCESS	= 3,	//���ӽ���
	CONNECT_LOST	= 4		//���Ӷ�ʧ
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
	CURL *curl;							//libcurl�Ķ���
	curl_socket_t sockfd;				//libcurl�򿪵�socket
	pthread_mutex_t m_mutexForVec;		//����m_sendPkgVec���߳���
	pthread_mutex_t m_mutexForSend;		//�������ݵ��߳���

	uint8* m_readBuffer;				//�������ݻ���������Ż�û���������İ�����
	uint8* m_writeBuffer;				//�������ݻ�������������ڷ��͵���������
	int m_readCursor;					//�������ݻ���������䳤��
	int m_writeCursor;					//���������ѷ��ͳ���
	int m_writeLen;						//��Ҫ�������ݵ��ܳ���
	volatile bool m_sendDirect;			//fd�Ƿ����ֱ�ӷ�������

	vector<WorldPacket*> m_sendPkgVec;	//���������ݰ�����

	const uint32 BUFFER_LEN;
	pthread_t m_threadInst;					//pthread�Ķ���
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
