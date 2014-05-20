#ifndef _GAME_MEDIATOR_H_
#define _GAME_MEDIATOR_H_

#include "cocos2d.h"
#include "Singleton.h"
#include "TileMap.h"
#include "GameScene.h"
#include "RoomScene.h"
#include "Connection.h"

class GameMediator:public Singleton<GameMediator>
{
public:
	GameMediator();
	~GameMediator() {};

public:
	Connection* m_connectInst;
	GameScene* m_gameScene;
	RoomScene* m_roomScene;
};

#endif //_GAME_MEDIATOR_H_


