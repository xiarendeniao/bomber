#ifndef __BOMB_H__
#define __BOMB_H__

#include "cocos2d.h"
#include "type.h"
#include "ByteBuffer.h"

class Bomb : public cocos2d::CCNode
{
public:
	Bomb(void);
	~Bomb(void);

	static Bomb* CreateBomb(bool serverControl = false);
	bool init(bool serverControl = false, uint8 redius = 1); //加上道具以后可以考虑爆炸范围增大 markbyxds 
	void DeadOnCallback();
	void _DeadOnCallback(bool serverControl = false);

private:
	cocos2d::CCAnimate *GetAnimate();
	void DeadDoneCallback();
	void CheckExplosionUpdate(float dt);

	cocos2d::CCSprite *sprite[5];//一个爆炸由5个炸点组成
	Behaviour behaviour;
	Direction direction;
};

#endif//__BOMB_H__