#ifndef _ROOM_SCENE_H_
#define _ROOM_SCENE_H_

#include "cocos2d.h"
#include "Connection.h"
#include <string>
using namespace std;
using namespace cocos2d;

struct PlayerInfo
{
	uint32 accountId;
	string accountName;
	uint8  ready;
	PlayerInfo(uint32 id,const string& name, uint8 ready):accountId(id), accountName(name), ready(ready) {}
};

typedef map<uint32, PlayerInfo*> PlayerMap;
struct RoomStruct
{
	uint16 roomId;
	uint16 mapId;
	uint8 maxCount;
	uint8 enterCount;
	PlayerMap m_playerMap;
	RoomStruct(uint16 roomId, uint16 mapId, uint8 maxCount, uint8 enterCount): roomId(roomId), mapId(mapId), maxCount(maxCount), enterCount(enterCount) {}
	~RoomStruct() {
		for (PlayerMap::iterator it = m_playerMap.begin(); it != m_playerMap.end(); it++)
			delete it->second;
	}
};

class RoomLayer;

typedef map<uint16, CCMenuItemFont*> RoomIdToMenu;
class RoomScene : public CCLayer
{
public:
	~RoomScene(void);

	static CCScene* scene();
	virtual bool init();
	CREATE_FUNC(RoomScene);

	void MenuReturnCallback(CCObject* pSender);
	void MenuNewRoomCallback(CCObject* pSender);
	void MenuLeaveRoomCallback(CCObject* pSender);
	void MenuRoomCallback(CCObject* pSender);
	void ChangeConnectStatus(CONNECT_STATUS status);
	void LeaveRoom(WorldPacket* pkg);
	void UpdateRoomList(WorldPacket* pkg);
	void UpdateRoomInfo(WorldPacket* pkg);

	void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent);
	void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent);

private:
	CCLabelTTF* m_connectLabel;	//��ʾ����״̬�ı�ǩ
	CCMenu* m_roomMenu;			//�����б�
	CCMenu* m_createMenu;		//��������İ�ť
	CCMenu* m_leaveMenu;		//�뿪����İ�ť
	CCPoint m_beginPos;			//�������

	RoomLayer* m_roomLayer;		//��ǰ��ʾ�ķ���
};

class RoomLayer : public CCLayer
{
public:
	RoomLayer(WorldPacket* pkg, RoomScene* roomScene);
	~RoomLayer();

	void MenuBodyCallback(CCObject* pSender);

//private:
	uint16 m_roomId;
	uint16 m_mapId;
	uint8 m_maxCount;
	uint8 m_enterCount;

	CCMenu* m_bodyMenu;
	RoomScene* m_roomScene;
};

#endif