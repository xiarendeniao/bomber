#include "Hero.h"
#include "type.h"
#include "tools.h"
#include "constant.h"
#include "AnimationManager.h"
#include "GameMediator.h"
#include "GameOverScene.h"
//#include "Connection.h"

using namespace cocos2d;

Hero::Hero()
{
}

Hero::~Hero()
{

}

Hero * Hero::hero(uint32 heroId)
{
	Hero *hero = new Hero();

	if (hero && hero->init())
	{
		hero->setHeroId(heroId);
		hero->autorelease();
		return hero;
	}
	CC_SAFE_DELETE(hero);
	return NULL;
}

bool Hero::init()
{
	bool bRet = false;
	do{
		this->setAnchorPoint(CCPointZero);

		//创建动画
		sprite = CCSprite::createWithSpriteFrameName("man_standdown0.png");//指定一张初始图片
		sprite->setAnchorPoint(CCPointZero);
		this->addChild(sprite);

		//设置状态
		behaviour=STAND;
		direction=DOWN;
		sprite->runAction(CCRepeatForever::create(getAnimate()));

		bRet=true;
	}while(0);

	return bRet;
}

//注意，不同精灵，这个函数写法不同，主要区别都是OVERLOOK上，有些精灵是没有四方向的
CCAnimate * Hero::getAnimate()
{
	if (behaviour==DEAD)
	{
		direction=OVERLOOK;
	}
	return AnimationManager::getInstance()->getAnimate("man",behaviour,direction);
}

//getPreviousLocation()和getLocation()拿到的是两个相隔比较近的点，比如：
//(111.0,222.0) -> (111.0,221.0) (111.0,221.0) -> (113.0,221.0) (143.0,202.0) -> (143.0,201.0) (162.0,186.0) -> (162.0,185.0) (173.0,173.0) -> (174.0,173.0)
//手机上操作时手指停在屏幕上不动的时候也会经常带来小距离的move事件,据getPreviousLocation()和getLocation()做移动判断会导致任务小范围乱跑 markbyxds 13.8.2
//滑动移动
void Hero::AcceptTouchesMoved(cocos2d::CCPoint& beginPos, cocos2d::CCPoint& endPos)
{
	if (behaviour==MOVE||behaviour==DEAD)
		return;

	//判断是否要响应滑动（滑动幅度达到阀值才会让hero移动）
	float deltaX = endPos.x - beginPos.x;
	float deltaY = endPos.y - beginPos.y;

	if (fabs(deltaX) < TILE_SIZE*0.4 && fabs(deltaY) < TILE_SIZE*0.4) {
		CCLog("move ignored for too short");
		return;
	}
	//移动的瓦片数
	int moveSize = fabs(deltaX)>=fabs(deltaY)? int(0.5+fabs(deltaX)/TILE_SIZE): int(0.5+fabs(deltaY)/TILE_SIZE);
	moveSize = moveSize < 1 ? 1 : moveSize;

	//CCPoint targetPosition;
	int i = 0;
	CCPoint curPos = this->getPosition();
	CCPoint tmpPos;
	for ( ; i < moveSize; i++){
		if (fabs(deltaX) >= fabs(deltaY))
			tmpPos = ccpAdd(curPos, ccp(deltaX>0?(i+1)*TILE_SIZE:-(i+1)*TILE_SIZE, 0));
		else 
			tmpPos = ccpAdd(curPos, ccp(0, deltaY>0?(i+1)*TILE_SIZE:-(i+1)*TILE_SIZE));
		if (!GameMediator::getInstance()->m_gameScene->CheckMovable(tmpPos))
			break;
	}
	if (i == 0)
		return;
	else
		moveSize = i;

	//移动
	Direction dir = OVERLOOK;
	if (fabs(deltaX)>=fabs(deltaY)) { //左右移动
		dir = deltaX > 0 ? RIGHT : LEFT;
	} else { //上下移动
		dir = deltaY > 0 ? UP : DOWN;
	}
	if (GameMediator::getInstance()->m_gameScene->IsRobotMap()) { //单机地图
		CCLog("to handle move (%.1f,%.1f) to (%.1f,%.1f)", beginPos.x, beginPos.y, endPos.x, endPos.y);
		Move(moveSize, dir);
	} else { //联网地图
		WorldPacket* pkg = new WorldPacket(CMSG_MOVE, 10);
		const CCPoint& curPos = GameMediator::getInstance()->m_gameScene->convertCoordGLToTile(this->getPosition());
		*pkg << (uint16)curPos.x << (uint16)curPos.y << (uint32)m_heroId << (uint8)moveSize << (uint8)dir;
		Connection* connectInst = GameMediator::getInstance()->m_connectInst;
		if (connectInst)
			connectInst->SendToServer(pkg);
		else
			CCLog("connection not found");
	}
}

void Hero::Move(uint8 step, Direction dir)
{
	uint32 delta = step * TILE_SIZE;
	CCPoint deltaPos = ccp(0,0);
	switch (dir)
	{
	case UP: //上
		deltaPos.y += delta;
		break;
	case DOWN: //下
		deltaPos.y -= delta;
		break;
	case LEFT: //左
		deltaPos.x -= delta;
		break;
	case RIGHT: //右
		deltaPos.x += delta;
		break;
	default:
		ASSERT(false);
	}
	CCPoint curPos = this->getPosition();
	CCPoint newPos = ccpAdd(curPos, deltaPos);
	CCFiniteTimeAction* moveToAction = CCMoveTo::create(0.3f*step, newPos);
	CCLog("hero %d moved (%.2f,%.2f) -> (%.2f,%.2f)", m_heroId, curPos.x, curPos.y, newPos.x, newPos.y);
	behaviour = MOVE;
	direction = dir;

	sprite->stopAllActions();
	CCAction *sequneceAction = CCSequence::create(moveToAction, CCCallFunc::create(this, callfunc_selector(Hero::moveDoneCallback)),NULL);
	this->runAction(sequneceAction);
	sprite->runAction(CCRepeatForever::create(getAnimate()));
}

void Hero::AcceptMoveCmd(int16 x, int16 y, uint8 step, uint8 dir)
{
	Move(step, (Direction)dir);
	return;

	CCPoint curPos = this->getPosition();
	if (curPos.x>x-0.0001 && curPos.x<x+0.0001 && curPos.y>y-0.0001 && curPos.y<y+0.0001) {
		//坐标校验完成
		Move(step, (Direction)dir);
	}else {
		CCLOG("hero position(%.2f,%.2f) not match SMSG_MOVE(%d, %d)", curPos.x, curPos.y, x, y);
	}
}

void Hero::moveDoneCallback()
{
	behaviour=STAND;
	sprite->stopAllActions();
	sprite->runAction(CCRepeatForever::create(getAnimate()));
}

void Hero::setDead()
{
	behaviour=DEAD;
	direction=OVERLOOK;

	this->cleanup();
	sprite->cleanup();
	
	CCAction *sequneceAction = CCSequence::create(
		getAnimate(),
		CCCallFunc::create(this, callfunc_selector(Hero::deadDoneCallback)),
		NULL);

	sprite->runAction(sequneceAction);
}

void Hero::deadDoneCallback()
{
	sprite->setVisible(false);
	GameMediator::getInstance()->m_gameScene->deleteNode(this);
}

cocos2d::CCRect Hero::getRect()
{
	return CCRectMake(this->getPosition().x,this->getPosition().y,TILE_SIZE-1,TILE_SIZE-1);
}

bool Hero::isDead()
{
	return behaviour==DEAD;
}
