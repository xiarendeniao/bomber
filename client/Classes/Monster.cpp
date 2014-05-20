#include "Monster.h"
#include "AnimationManager.h"
#include "constant.h"
#include "GameMediator.h"

using namespace cocos2d;

Monster::Monster(void)
{
}

Monster::~Monster(void)
{
}

Monster* Monster::monster()
{
	Monster *monster=new Monster;

	if (monster&&monster->init())
	{
		monster->autorelease();
		return monster;
	}

	return NULL;
}

bool Monster::init()
{
	bool bRet = false;
	do{
		this->setAnchorPoint(CCPointZero);
		
		spirte=CCSprite::create("monster_move0.png");
		spirte->setAnchorPoint(CCPointZero);
		this->addChild(spirte);

		behaviour=STAND;
		direction=DOWN;

		CCActionInterval *action=getAnimate(MOVE,OVERLOOK);
		spirte->runAction(CCRepeatForever::create(action));

		this->schedule(schedule_selector(Monster::moveUpdate));

		bRet=true;
	}while(0);
	return bRet;
}

//注意，不同精灵，这个函数写法不同，主要区别都是OVERLOOK上，有些精灵是没有四方向的
cocos2d::CCAnimate * Monster::getAnimate()
{
	direction=OVERLOOK;
	return AnimationManager::getInstance()->getAnimate("monster",behaviour,direction);
}

cocos2d::CCAnimate* Monster::getAnimate( Behaviour behaviour,Direction direction )
{
	return AnimationManager::getInstance()->getAnimate("monster",behaviour,direction);
}

void Monster::moveUpdate(float dt)
{
	if (behaviour == MOVE)
		return;

	int value = abs(rand()%6);
	switch(value)
	{
		case 5:
		case 4:
			break;
		default:
			direction = (Direction)value;
	}

	CCPoint deltaPoint;
	switch(direction)
	{
	case UP:
		deltaPoint = ccp(0,TILE_SIZE);
		break;
	case DOWN:
		deltaPoint = ccp(0,-TILE_SIZE);
		break;
	case LEFT:
		deltaPoint = ccp(-TILE_SIZE,0);
		break;
	case RIGHT:
		deltaPoint = ccp(TILE_SIZE,0);
		break;
	}

	CCPoint targetPosition = ccpAdd(this->getPosition(),deltaPoint);
	CCLog("attempt to move from (%.2f,%.2f) to (%.2f,%.2f)", this->getPosition().x, this->getPosition().y, targetPosition.x, targetPosition.y);
	if (!GameMediator::getInstance()->m_gameScene || !GameMediator::getInstance()->m_gameScene->CheckMovable(targetPosition))
		return;

	behaviour=MOVE;
	CCFiniteTimeAction *moveByAction = CCMoveBy::create(0.7f,deltaPoint);
	CCAction *sequneceAction = CCSequence::create(
		moveByAction,
		CCCallFunc::create(this, callfunc_selector(Monster::moveDoneCallback)),
		NULL);

	this->runAction(sequneceAction);
}

void Monster::moveDoneCallback()
{
	behaviour = STAND;
}

void Monster::deadDoneCallback()
{
	GameMediator::getInstance()->m_gameScene->deleteNode(this);
}

void Monster::setDead()
{
	behaviour = DEAD;

	CCActionInterval *deadAction=getAnimate(DEAD,OVERLOOK);
	CCAction *sequneceAction = CCSequence::create(
		deadAction,
		CCCallFunc::create(this, callfunc_selector(Monster::deadDoneCallback)),
		NULL);

	this->cleanup();
	spirte->cleanup();
	spirte->runAction(sequneceAction);
}

bool Monster::isDead()
{
	return behaviour==DEAD;
}

cocos2d::CCRect Monster::getRect()
{
	return CCRectMake(this->getPosition().x,this->getPosition().y,TILE_SIZE-1,TILE_SIZE-1);
}
