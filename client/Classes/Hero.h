#ifndef __HERO_H__
#define __HERO_H__

#include "cocos2d.h"
#include "type.h"
#include "ByteBuffer.h"
#include <map>

class Hero : public cocos2d::CCNode
{
public:
	Hero();
	~Hero();
	static Hero *hero(uint32 heroId);
	bool init();

	void AcceptTouchesMoved( cocos2d::CCPoint& beginPos, cocos2d::CCPoint& endPos);
	void AcceptMoveCmd(int16 x, int16 y, uint8 step, uint8 dir);

	void setDead();
	bool isDead();
	cocos2d::CCRect getRect();
	inline uint32 getHeroId() { return m_heroId; }
	inline void setHeroId(uint32 id) { m_heroId = id; }

private:
	void Move(uint8 step, Direction dir);
	cocos2d::CCAnimate *getAnimate();
	void moveDoneCallback();
	void deadDoneCallback();

	uint32 m_heroId;
	cocos2d::CCSprite *sprite;
	Behaviour behaviour;
	Direction direction;
};

typedef std::map<uint32, Hero*> HeroMap;

#endif //__HERO_H__