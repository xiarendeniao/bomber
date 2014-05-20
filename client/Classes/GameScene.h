#ifndef _GAME_SCENE_H_
#define _GAME_SCENE_H_

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "ByteBuffer.h"
#include "Hero.h"
#include "Connection.h"

class Bomb;
class Monster;
class TileMap;

using namespace cocos2d;

class GameScene : public CCLayer
{
public:
	~GameScene();
	static CCScene* scene(uint16 mapId = 0, uint16 roleId = 0);
	virtual bool init();
	CREATE_FUNC(GameScene);

	bool _init(uint16 mapId, uint16 roleId);

	virtual void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent);

	void setSceneScrollPosition(float dt);
	bool IsRobotMap();
	Hero* FetchHeroById(uint16 id);
	void UpdateDebugInfo(const char* str);
	bool ToMineBomb(const cocos2d::CCPoint& position);
	bool DoMineBomb(const cocos2d::CCPoint& position);
	bool deleteNode(cocos2d::CCNode *node);

	bool CanBomb(const CCPoint& pos);
	void CheckExplosion(CCPoint position);
	void CheckGameOver(float dt);
	void CheckHeroDead(float dt);
	bool CheckMovable(CCPoint position);
	void ChangeConnectStatus(CONNECT_STATUS status);

	//server trigger
	void MineExplode(const cocos2d::CCPoint& position, uint8 radius);
	void RoleDead(uint16 roleId);
	void GameOver(uint8 result);

	CCPoint convertCoordGLToTile(const cocos2d::CCPoint &glpoint);
	CCPoint convertTileToCoordGL(const cocos2d::CCPoint& tilePoint);

private:
	bool collisionWithBombs(cocos2d::CCPoint position);

private:
	CCPoint m_touchBeginPos;		//点击开始的位置
	CCPoint m_touchEndPos;			//点击结束的位置（两点确定滑动的距离）
	long m_touchTime;				//点击时间，根据时间间隔判断是否放雷

	HeroMap m_heroMap;
	uint16 m_heroId;
	TileMap *m_tileMap;
	CCLabelTTF *m_debugLabel;
	std::string m_mapFile;

	std::vector<Bomb*> bombs;
	std::vector<Monster*> monsters;
};

#endif
