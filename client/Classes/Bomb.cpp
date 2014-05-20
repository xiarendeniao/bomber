#include "Bomb.h"
#include "AnimationManager.h"
#include "GameMediator.h"
#include "constant.h"
#include <vector>
#include <algorithm>

using namespace  cocos2d;
using namespace std;

Bomb::Bomb(void)
{
}

Bomb::~Bomb(void)
{
}

Bomb* Bomb::CreateBomb(bool serverControl)
{
	Bomb *bomb = new Bomb;

	if (bomb && bomb->init(serverControl))
	{
		bomb->autorelease();
		return bomb;
	}

	return NULL;
}

bool Bomb::init(bool serverControl, uint8 radius)
{
	bool bRet = false;
	do{
		this->setAnchorPoint(CCPointZero);

		for(int i = 0; i < 5; i++){
			sprite[i] = CCSprite::createWithSpriteFrameName("bomb_stand0.png");//指定一张初始图片
			sprite[i]->setAnchorPoint(CCPointZero);
			sprite[i]->setVisible(false);
			this->addChild(sprite[i]);
		}
		CCPoint position=this->getPosition();
		sprite[1]->setPosition(ccp(position.x-TILE_SIZE,position.y));
		sprite[2]->setPosition(ccp(position.x+TILE_SIZE,position.y));
		sprite[3]->setPosition(ccp(position.x,position.y-TILE_SIZE));
		sprite[4]->setPosition(ccp(position.x,position.y+TILE_SIZE));

		sprite[0]->setVisible(true);
		behaviour = STAND;
		direction = OVERLOOK;
		CCFiniteTimeAction *standAction = GetAnimate();
		CCCallFunc* callback = NULL;
		if (!serverControl)
			callback = CCCallFunc::create(this, callfunc_selector(Bomb::DeadOnCallback));
		CCAction* sequneceAction = CCSequence::create( CCRepeat::create(standAction,3), callback, NULL);
		sprite[0]->runAction(sequneceAction);

		bRet=true;
	}while(0);
	return bRet;
}

//注意，不同精灵，这个函数写法不同，主要区别都是OVERLOOK上，有些精灵是没有四方向的
cocos2d::CCAnimate * Bomb::GetAnimate()
{
	direction=OVERLOOK;
	return AnimationManager::getInstance()->getAnimate("bomb",behaviour,direction);
}

void Bomb::DeadDoneCallback()
{
	GameMediator::getInstance()->m_gameScene->deleteNode(this);
}

void Bomb::_DeadOnCallback(bool serverControl)
{
	behaviour = DEAD;
	direction = OVERLOOK;

	for(int i = 1; i < 5; i++){
		CCLog("expode at pos (%.1f,%.1f)", sprite[i]->getPosition().x, sprite[i]->getPosition().y);
		sprite[i]->setVisible(true);
		CCFiniteTimeAction *deadAction = GetAnimate();
		sprite[i]->runAction(deadAction);
	}

	CCFiniteTimeAction *deadAction = GetAnimate();
	CCCallFunc* callback = CCCallFunc::create(this, callfunc_selector(Bomb::DeadDoneCallback));
	CCAction *sequneceAction = CCSequence::create(deadAction, callback, NULL);
	sprite[0]->runAction(sequneceAction);

	this->schedule(schedule_selector(Bomb::CheckExplosionUpdate), 0.05f);
}

void Bomb::DeadOnCallback()
{
	this->_DeadOnCallback(false);
}	

void Bomb::CheckExplosionUpdate(float dt)
{
	GameMediator::getInstance()->m_gameScene->CheckExplosion(this->getPosition());
}
