#ifndef _MONSTER_H_
#define _MONSTER_H_

#include "cocos2d.h"
#include "type.h"

class Monster: public cocos2d::CCNode
{
public:
	Monster(void);
	~Monster(void);

	static Monster* monster();
	bool init();

	void setDead();
	bool isDead();
	cocos2d::CCRect getRect();
private:
	cocos2d::CCAnimate* getAnimate();
	cocos2d::CCAnimate* getAnimate(Behaviour behaviour,Direction direction);

	void moveUpdate(float dt);
	void moveDoneCallback();
	void deadDoneCallback();

	cocos2d::CCSprite *spirte;
	Behaviour behaviour;
	Direction direction;	
};

#endif//_MONSTER_H_

