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
	bool init(bool serverControl = false, uint8 redius = 1); //���ϵ����Ժ���Կ��Ǳ�ը��Χ���� markbyxds 
	void DeadOnCallback();
	void _DeadOnCallback(bool serverControl = false);

private:
	cocos2d::CCAnimate *GetAnimate();
	void DeadDoneCallback();
	void CheckExplosionUpdate(float dt);

	cocos2d::CCSprite *sprite[5];//һ����ը��5��ը�����
	Behaviour behaviour;
	Direction direction;
};

#endif//__BOMB_H__