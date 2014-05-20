#include "Connection.h"
#include "cocos2d.h"
#include "WorldPacket.h"
#include "GameMediator.h"
#include "GameOverScene.h"
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
#include <errno.h>
#include <arpa/inet.h>
#endif

using namespace cocos2d;

#pragma pack(push, 1)
struct PacketHeader
{
	uint16 size;
	uint16 cmd;
};
#pragma pack(pop)

void* ThreadFunction(void* arg)
{
	CCLog("i am in network thread.");
	Connection* connObj = (Connection*)arg;
	connObj->Run();
	return NULL;
}

Connection::Connection(void): BUFFER_LEN(1024*1024), m_sendDirect(true), m_writeLen(0), m_writeCursor(0), m_readCursor(0)
{
	curl = NULL;
	m_readBuffer = NULL;
	m_writeBuffer = NULL;

	pthread_mutex_init(&m_mutexForVec, NULL);
	pthread_mutex_init(&m_mutexForSend, NULL);
	//m_mutexForVec = PTHREAD_MUTEX_INITIALIZER;
	//m_mutexForSend = PTHREAD_MUTEX_INITIALIZER;
	pthread_create(&m_threadInst, NULL, &ThreadFunction, this);
	m_readBuffer = new uint8[BUFFER_LEN];
	m_writeBuffer = new uint8[BUFFER_LEN];
	m_connectStatus = CONNECT_INIT;
	m_threadToExit = false;
}

Connection::~Connection(void)
{
	if (curl)
		curl_easy_cleanup(curl);
	if (m_readBuffer)
		delete[] m_readBuffer;
	if (m_writeBuffer)
		delete[] m_writeBuffer;
	vector<WorldPacket*> m_sendPkgVec;
	for (vector<WorldPacket*>::iterator it = m_sendPkgVec.begin(); it != m_sendPkgVec.end(); ) {
		delete *it;
		it = m_sendPkgVec.erase(it);
	}
}

void Connection::Run()
{
	//连接服务器
	ChangeConnectStatus(CONNECT_ING);
	char* errInfo = NULL;
	if (!ConnectToServer(&errInfo)) {
		CCLog("connect to server failed.");
		if (errInfo)
			delete[] errInfo;
		ChangeConnectStatus(CONNECT_FAILED);
		return;
	}else {
		ChangeConnectStatus(CONNECT_SUCCESS);
	}

	DEBUG_SIMULATE_SMSG();

	//数据收发
	CURLcode curlRes;
	size_t iolen;
	curl_off_t nread;

	struct timeval tv;
	fd_set infd, outfd, errfd;
	int res;
	tv.tv_sec = 0;
	tv.tv_usec = 100;

	CONNECT_STATUS newstatus = m_connectStatus;
	while (!m_threadToExit) {
		FD_ZERO(&outfd);
		FD_ZERO(&infd);
		FD_ZERO(&errfd);
		FD_SET(sockfd, &errfd);
		FD_SET(sockfd, &infd);
		if (!m_sendDirect || m_sendPkgVec.size()>0)
			FD_SET(sockfd, &outfd);
		res = select(sockfd + 1, &infd, &outfd, &errfd, &tv);
		if (res < 0) {
			CCLog("select returned %d", res);
			continue;
		} else if (res == 0) {
			//CCLog("select timeout.");
			continue;
		} else {
			//读数据
			if (FD_ISSET(sockfd, &infd)) {
				ASSERT(m_readCursor < BUFFER_LEN);
				curlRes = curl_easy_recv(curl, m_readBuffer+m_readCursor, BUFFER_LEN-m_readCursor, &iolen);
				if (curlRes != CURLE_OK) {
					CCLog("curl_easy_recv error: %s", curl_easy_strerror(curlRes));
					newstatus = CONNECT_LOST;
					break;
				}
				nread = (curl_off_t)iolen;
				m_readCursor += iolen;
				CCLog("received %" CURL_FORMAT_CURL_OFF_T " bytes", nread);

				PacketHeader header;
				while (m_readCursor >= sizeof(PacketHeader)) { //包头已经读出来了
					memcpy((uint8*)&header, m_readBuffer, sizeof(PacketHeader));
					header.size = ntohs(header.size); //字节序，从网络大头转成本机小头
					uint32 pkgSize = header.size; //cmd+data的长度，不包含数据包最开始的2字节（也就是存储size本身的字段）
					uint16 opcode = header.cmd;
					if (pkgSize <= m_readCursor - sizeof(header.size)) { //当前包的数据已经完整收到了
						CCLog("received package %d (%d bytes)", header.cmd, header.size + sizeof(header.size));
						WorldPacket* pkg = new WorldPacket(opcode, pkgSize - sizeof(header.cmd));
						pkg->resize(pkgSize - sizeof(header.cmd));
						memcpy((uint8*)(pkg->contents()), m_readBuffer+sizeof(header), pkgSize - sizeof(header.cmd));
						PublicMsgHandler::getInstance()->addPacket(pkg);
						//修正buff的偏移（粘包）
						uint32 newCursor = m_readCursor-pkgSize-sizeof(header.size);
						memmove(m_readBuffer, m_readBuffer+pkgSize+sizeof(header.size), newCursor); //有优化的空间（内存可以做到不copy） markbyxds 
						m_readCursor = newCursor;
					} else { //当前包的数据还没收完整
						break;
					}
				}
			}
			//写数据
			if (FD_ISSET(sockfd, &outfd)) {
				pthread_mutex_lock(&m_mutexForSend);
				if (m_writeLen == 0) { //没有数据需要发送
					m_sendDirect = true;
					pthread_mutex_unlock(&m_mutexForSend);
					continue;
				} else {
					m_sendDirect = false;
				}
				curlRes = curl_easy_send(curl, m_writeBuffer+m_writeCursor, m_writeLen-m_writeCursor, &iolen);
				pthread_mutex_unlock(&m_mutexForSend);
				if (curlRes != CURLE_OK) {
					CCLog("send data error: %s", curl_easy_strerror(curlRes));
					newstatus = CONNECT_LOST;
					break;
				}else {
					m_writeCursor += iolen;
					if (m_writeCursor >= m_writeLen) {//数据发送完成
						CCLog("sent a compete package");
						m_writeLen = 0;
						m_writeCursor = 0;
						if (m_sendPkgVec.size() > 0) {
							//取新的数据包做发送
							pthread_mutex_lock(&m_mutexForVec);
							WorldPacket* pkg = *m_sendPkgVec.begin();
							PacketHeader header;
							header.size = pkg->size() + sizeof(header.cmd);
							header.cmd = pkg->GetOpcode();
							m_writeLen = pkg->size() + sizeof(header);
							memcpy(m_writeBuffer, &header, sizeof(header));
							memcpy(m_writeBuffer+sizeof(header), (uint8*)pkg->contents(), pkg->size());
							//把新的数据包删掉
							m_sendPkgVec.erase(m_sendPkgVec.begin());
							pthread_mutex_unlock(&m_mutexForVec);
						}
					}
				}
			}
			//ERROR
			if (FD_ISSET(sockfd, &errfd)) {
				CCLog("error occored.");
				newstatus = CONNECT_LOST;
				break;
			}
		}
	}
	if (newstatus != m_connectStatus)
		ChangeConnectStatus(newstatus);
}

bool Connection::ConnectToServer(char** errInfo)
{
	CURLcode res;
	long sockextr;

	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		//curl_easy_setopt(curl, CURLOPT_URL, "192.168.1.212");
		//curl_easy_setopt(curl, CURLOPT_URL, "172.26.16.100");
		curl_easy_setopt(curl, CURLOPT_URL, "www.xiaoxiandi.com");
		curl_easy_setopt(curl, CURLOPT_PORT, 9999);
		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
		res = curl_easy_perform(curl);
		if (res == CURLE_OK)
			res = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &sockextr);
		if (res != CURLE_OK) {
			const char* err = curl_easy_strerror(res);
			CCLog("libcurl connect to server failed: %s", err);
			if (errInfo != NULL) {
				int strSize = strlen(err);
				*errInfo = new char[strSize+1];
				strcpy(*errInfo, err);
			}
			return false;
		}else {
			sockfd = sockextr;
			return true;
		}
	}else
		return false;
}

void Connection::StopNetwork()
{
	int kill_rc = pthread_kill(m_threadInst,0);
	if(kill_rc == ESRCH) {
		CCLog("network thread does not exist");
	} else if(kill_rc == EINVAL) {
		CCLog("signal is invalid");
	} else {
		CCLog("network thread is alive");
		m_threadToExit = true;
		/*int cancel_rc = pthread_cancel(m_threadInst);
		if (cancel_rc == 0) {
		CCLog("cancel network thread sucess");
		} else if (cancel_rc == ESRCH) {
		CCLog("network thread does not exist");
		} else {
		CCAssert(false, "cancel returned invalid code");
		}*/
	}
	CCLog("waiting network to exit...");
	pthread_join(m_threadInst, NULL);
	CCLog("network thread exited");
}

void Connection::ChangeConnectStatus(CONNECT_STATUS status)
{
	m_connectStatus = status;
	switch (status)
	{
	case CONNECT_INIT:
		break;
	case CONNECT_ING:
	case CONNECT_FAILED:
	case CONNECT_SUCCESS:
	case CONNECT_LOST:
		{
			WorldPacket* pkg = new WorldPacket(MMSG_CONNECT_STATUS, 1);
			*pkg << (uint8)status;
			PublicMsgHandler::getInstance()->addPacket(pkg);
			break;
		}
	default:
		{
			CCAssert(false, "unrecognized status");
			break;
		}
	}
}

void Connection::SendToServer(WorldPacket* pkg)
{
	pthread_mutex_lock(&m_mutexForSend);
	if (m_sendPkgVec.size() == 0 && m_writeLen == 0) {
		ASSERT(m_sendDirect == true);
		PacketHeader header;
		header.size = pkg->size() + sizeof(header.cmd);
		header.size = htons(header.size); //字节序，从本机的小头转换成大头
		header.cmd = pkg->GetOpcode();
		m_writeLen = pkg->size() + sizeof(header);
		memcpy(m_writeBuffer, &header, sizeof(header));
		memcpy(m_writeBuffer+sizeof(header), (uint8*)pkg->contents(), pkg->size());
		delete pkg;
		size_t iolen;
		CURLcode curlRes = curl_easy_send(curl, m_writeBuffer+m_writeCursor, m_writeLen-m_writeCursor, &iolen);
		if (curlRes != CURLE_OK) {
			CCLog("send data error: %s", curl_easy_strerror(curlRes));
		}else {
			m_writeCursor += iolen;
			if (m_writeCursor >= m_writeLen) { //数据发送完成
				CCLog("sent a compete package");
				m_writeLen = 0;
				m_writeCursor = 0;
			}else { //数据没发送完
				m_sendDirect = false;
			}
		}
		pthread_mutex_unlock(&m_mutexForSend);
	} else {
		pthread_mutex_unlock(&m_mutexForSend);
		pthread_mutex_lock(&m_mutexForVec);
		m_sendPkgVec.push_back(pkg);
		pthread_mutex_unlock(&m_mutexForVec);
	}
}

class DebugLayer: public CCLayer
{
public:
	CREATE_FUNC(DebugLayer);
	virtual bool init();  
};

bool DebugLayer::init()
{
	if ( !CCLayer::init() )
	{
		return false;
	}
	CCLabelTTF* pLabel = CCLabelTTF::create("PublicMsgHandler deconstructed..", "Thonburi", 30);
	if (!pLabel)
		return false;
	pLabel->setColor(ccRED);

	CCSize size = CCDirector::sharedDirector()->getWinSize();
	pLabel->setPosition(ccp(size.width/2, size.height/2));
	this->addChild(pLabel);
}

PublicMsgHandler::PublicMsgHandler()
{
	pthread_mutex_init(&mutex, NULL);
	//mutex = PTHREAD_MUTEX_INITIALIZER;
	this->onEnter();
	this->onEnterTransitionDidFinish();
	this->scheduleUpdate();
	CCLog("PlubicMsgHandler constructed..");
}

PublicMsgHandler::~PublicMsgHandler()
{
	CCDirector::sharedDirector()->getRunningScene()->addChild(DebugLayer::create());

	this->unscheduleUpdate();
	if (m_pkgVec.size() > 0) {
		for (vector<WorldPacket*>::iterator it = m_pkgVec.begin(); it != m_pkgVec.end(); ) {
			delete *it;
			it = m_pkgVec.erase(it);
		}
	}
}

void PublicMsgHandler::update(float fDelta){
	//return;
	WaitMutex();
	if (m_pkgVec.size() > 0) {
		for (vector<WorldPacket*>::iterator it = m_pkgVec.begin(); it != m_pkgVec.end(); ) {
			WorldPacket* msg = *it;
			//处理数据包
			uint16 opcode = msg->GetOpcode();
			switch (opcode)
			{
			case SMSG_MOVE:
				{
					//解析数据包
					Location loc;
					uint32 unitId;
					uint8 step;
					uint8 dir;
					*msg >> loc.x >> loc.y >> unitId >> step >> dir;
					//控制精灵移动
					if (!GameMediator::getInstance()->m_gameScene) {
						CCLog("for SMSG_MOVE, game scene not found yet");
					} else {
						CCPoint pos = GameMediator::getInstance()->m_gameScene->convertTileToCoordGL(ccp(loc.x,loc.y));
						Hero* hero = GameMediator::getInstance()->m_gameScene->FetchHeroById(unitId);
						if (hero)
							hero->AcceptMoveCmd(pos.x, pos.y, step, dir);
					}
					break;
				}
			case MMSG_CONNECT_STATUS:
				{
					uint8 status;
					string info;
					*msg >> status >> info;
					if (GameMediator::getInstance()->m_roomScene) { //还没进入地图
						GameMediator::getInstance()->m_roomScene->ChangeConnectStatus((CONNECT_STATUS)status);
					} else if (GameMediator::getInstance()->m_gameScene) { //处于地图场景中
						GameMediator::getInstance()->m_gameScene->ChangeConnectStatus((CONNECT_STATUS)status);
					}
					break;
				}
			case SMSG_ROOM_LIST:
				{
					RoomScene* roomScene = GameMediator::getInstance()->m_roomScene;
					if (!roomScene) {
						CCLog("room scene not found");
						break;
					}
					roomScene->UpdateRoomList(msg);
					break;
				}
			case SMSG_ROOM_INFO:
				{
					RoomScene* roomScene = GameMediator::getInstance()->m_roomScene;
					if (!roomScene) {
						CCLog("room scene not found");
						break;
					}
					roomScene->UpdateRoomInfo(msg);
					break;
				}
			case SMSG_ROOM_LEAVE:
				{
					RoomScene* roomScene = GameMediator::getInstance()->m_roomScene;
					if (!roomScene) {
						CCLog("room scene not found");
						break;
					}
					roomScene->LeaveRoom(msg);
					break;
				}
			case SMSG_MAP_ENTER:
				{
					uint16 mapId, roleId;
					*msg >> mapId >> roleId;
					CCScene *scene = GameScene::scene(mapId, roleId);
					GameMediator::getInstance()->m_roomScene->cleanup();
					GameMediator::getInstance()->m_roomScene = NULL;
					CCDirector::sharedDirector()->replaceScene(CCTransitionCrossFade::create(1.2f,scene));
					break;
				}
			case SMSG_BOMB_BORN:
				{
					uint16 x, y;
					*msg >> x >> y;
					GameScene* gameScene = GameMediator::getInstance()->m_gameScene;
					if (!gameScene) {
						CCLog("game scene not found");
						break;
					}
					gameScene->DoMineBomb(gameScene->convertTileToCoordGL(ccp(x,y)));
					break;
				}
			case SMSG_BOMB_EXPLODE:
				{
					uint16 x, y, radius;
					*msg >> x >> y >> radius;
					GameScene* gameScene = GameMediator::getInstance()->m_gameScene;
					if (!gameScene) {
						CCLog("game scene not found");
						break;
					}
					gameScene->MineExplode(gameScene->convertTileToCoordGL(ccp(x,y)), radius);
					break;
				}
			case SMSG_ROLE_DEAD:
				{
					GameScene* gameScene = GameMediator::getInstance()->m_gameScene;
					if (!gameScene) {
						CCLog("game scene not found");
						break;
					}
					uint8 count;
					uint16 roleId;
					*msg >> count;
					for (uint8 i = 0; i < count; i++) {
						*msg >> roleId;
						gameScene->RoleDead(roleId);
					}
					break;
				}
			case SMSG_GAME_OVER:
				{
					GameScene* gameScene = GameMediator::getInstance()->m_gameScene;
					if (!gameScene) {
						CCLog("game scene not found");
						break;
					}
					uint8 win;
					*msg >> win;
					gameScene->GameOver(win);
				}
			default:
				{
					CCLog("unrecognized opcode %d", opcode);
					break;
				}
			}
			//释放内存，从数据包数组中删除该包
			delete msg;
			it = m_pkgVec.erase(it);
		}
	}
	ClearMutex();
}

void PublicMsgHandler::addPacket(WorldPacket* pkg)
{
	//return;
	WaitMutex();
	m_pkgVec.push_back(pkg);
	ClearMutex();
}

void Connection::DEBUG_SIMULATE_SMSG()
{
//#define SINGLE_DEBUG
#ifdef SINGLE_DEBUG
	WorldPacket* pkg = NULL;
	{
		pkg = new WorldPacket(SMSG_ROOM_INFO, 12);
		uint16 roomId = 1, mapId = 1;
		uint8 maxCount = 6, enterCount = 3;
		*pkg << roomId << mapId << maxCount << enterCount;
		char buff[32];
		for (uint8 i = 0; i < enterCount; i++) {
			int v = rand()%100;
			uint32 accountId = 100000 + v; 
			sprintf(buff, "robot-%d", v);
			string accountName = buff;
			uint8 ready = rand()%2;
			*pkg << accountId << accountName << ready;
		}
		if (pkg)
			PublicMsgHandler::getInstance()->addPacket(pkg);
	}

	pkg = NULL;
	{
		pkg = new WorldPacket(SMSG_ROOM_LIST, 12);
		uint16 roomSum = 40;
		*pkg << roomSum; 
		for (uint16 i = 0; i < roomSum; i++) {
			*pkg << (uint16)(i+1) << (uint8)6 << (uint8)3;
		}
		if (pkg)
			PublicMsgHandler::getInstance()->addPacket(pkg);
	}

	Sleep(5000);
	pkg = NULL;
	{
		pkg = new WorldPacket(SMSG_MAP_ENTER, 8);
		uint16 mapId = 2001, roleId = 1;
		*pkg << mapId << roleId;
		if (pkg)
			PublicMsgHandler::getInstance()->addPacket(pkg);
	}

	Sleep(5000);
	pkg = NULL;
	{
		pkg = new WorldPacket(SMSG_MOVE, 12);
		*pkg << (uint16)16 << (uint16)288 << (uint32)1 << (uint8)5 << (uint8)3;
		if (pkg)
			PublicMsgHandler::getInstance()->addPacket(pkg);
	}
#endif
}