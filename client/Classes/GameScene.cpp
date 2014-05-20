#include "GameScene.h"
#include "tools.h"
#include "Hero.h"
#include "Bomb.h"
#include "Monster.h"
#include "AnimationManager.h"
#include "type.h"
#include "constant.h"
#include "GameMediator.h"
#include "TileMap.h"
#include "GameOverScene.h"

using namespace cocos2d;

#define ROBOT_MAP_MAX_ID 2000 //单机地图的最大ID（ID超过该值的都是联机地图）

//"robot_*" 单机地图
//"pk_*" 联网地图

bool GameScene::IsRobotMap()
{
	if (this->m_mapFile.find("robot_") == 0)
		return true;
	else if (this->m_mapFile.find("pk_") == 0)
		return false;
	else {
		CCAssert(false, "map type unrecognized.");
		return false;
	}
}

cocos2d::CCScene* GameScene::scene(uint16 mapId, uint16 roleId)
{
	CCScene *scene = CCScene::create();
	GameScene *layer = GameScene::create();
	layer->_init(mapId, roleId);
	scene->addChild(layer);
	return scene;
}

bool GameScene::init()
{
	if ( !CCLayer::init() )
	{
		return false;
	}
	return true;
}

bool GameScene::_init(uint16 mapId, uint16 roleId)
{
	char buff[32];
	if (mapId <= ROBOT_MAP_MAX_ID)
		sprintf(buff, "robot_%d.tmx", mapId);
	else
		sprintf(buff, "pk_%d.tmx", mapId);
	this->m_mapFile = buff;
	this->m_heroId = roleId;

	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	//瓦片地图
	m_tileMap = TileMap::initTileMap(this->m_mapFile.c_str());
	this->addChild(m_tileMap, 1);
	CCSize mapSize = m_tileMap->getContentSize();

	//地图底色
	GLfloat width = mapSize.width;
	GLfloat height = mapSize.height;
	CCLayerColor *layerColor = CCLayerColor::create(ccc4(0,151,0,255),width,height);
	this->addChild(layerColor,0);

	//加载动画资源
	AnimationManager::getInstance()->loadAnimation(heroAnimation,countOf(heroAnimation));
	AnimationManager::getInstance()->loadAnimation(bombAnimation,countOf(bombAnimation));
	AnimationManager::getInstance()->loadAnimation(monsterAnimation,countOf(monsterAnimation));

	//主角
	//获取对象组
	CCTMXObjectGroup *objectGroup=m_tileMap->objectGroupNamed("Objects1");
	CCAssert(objectGroup!=NULL,"Objects' object group not found");

	//精灵
	CCArray* heroDictionaries = objectGroup->getObjects();
	for(unsigned int i = 0; i < heroDictionaries->count(); i++)
	{
		CCDictionary *heroDictionary = (CCDictionary*)heroDictionaries->objectAtIndex(i);
		std::string name = heroDictionary->valueForKey("name")->getCString();
		if (name == std::string("man"))
		{
			uint32 heroId = heroDictionary->valueForKey("roleId")->uintValue();
			//解析属性，注意！这里直接返回的是——相对于0，0点的点坐标（x，y），而tiled编辑器中指定的是（i，j）
			float x = heroDictionary->valueForKey("x")->floatValue();
			float y = heroDictionary->valueForKey("y")->floatValue();
			Hero *hero = Hero::hero(heroId);
			this->addChild(hero,2);
			hero->setPosition(ccp(x,y));
			m_heroMap[hero->getHeroId()] = hero;
		}
	}

	//怪物
	CCArray* monsterDictionaries = objectGroup->getObjects();
	for(unsigned int i = 0; i < monsterDictionaries->count(); i++)
	{
		CCDictionary *monsterDictionary = (CCDictionary*)monsterDictionaries->objectAtIndex(i);
		std::string name = monsterDictionary->valueForKey("name")->getCString();
		if (name == std::string("monster"))				
		{				
			float x = monsterDictionary->valueForKey("x")->floatValue();
			float y = monsterDictionary->valueForKey("y")->floatValue();
			Monster *monster = Monster::monster();
			this->addChild(monster,2);
			monster->setPosition(ccp(x,y));
			monsters.push_back(monster);
		}
	}

	//DEBUG窗口
	m_debugLabel = CCLabelTTF::create("", "Arial", 24, CCSize(500,20), kCCTextAlignmentLeft);
	m_debugLabel->setAnchorPoint(CCPointZero);
	m_debugLabel->setPosition(ccp(2, 2));
	m_debugLabel->setColor(ccRED);
	this->addChild(m_debugLabel,2);

	//注册
	GameMediator::getInstance()->m_gameScene = this;

	this->schedule(schedule_selector(GameScene::setSceneScrollPosition));
	if (IsRobotMap()) {
		this->schedule(schedule_selector(GameScene::CheckHeroDead));
		this->schedule(schedule_selector(GameScene::CheckGameOver));
	}

	//开启触摸
	this->setTouchEnabled(true);
	return true;
}

//由于这不是触摸精灵的关系（而是触摸屏幕），所以我们通过转发把信息处理权交给hero
void GameScene::ccTouchesBegan( cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent )
{	
	//记录滑动的起始位置
	CCTouch* touch = (CCTouch*)*(pTouches->begin());
	m_touchBeginPos = touch->getLocation();
	CCLog("touch begin pos (%.1f,%.1f)", m_touchBeginPos.x, m_touchBeginPos.y);
}

void GameScene::ccTouchesMoved( cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent )
{
	//m_heroMap[m_heroId]->acceptTouchesMoved(pTouches,pEvent);
}

void GameScene::ccTouchesEnded( cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent )
{
	//判断是否要放雷
	long time = millisecondNow();
	if (m_touchTime > 0 && time - m_touchTime < 250) {
		GameMediator::getInstance()->m_gameScene->ToMineBomb(m_heroMap[m_heroId]->getPosition());
		m_touchTime = 0;
	} else {
		m_touchTime = time;
	}

	//记录滑行的结束位置
	CCSetIterator lastIt = pTouches->begin();
	for (CCSetIterator it = pTouches->begin(); ; it++) {
		if (it != pTouches->end())
			lastIt = it;
		else
			break;
	}
	CCTouch* touch = (CCTouch*)(*lastIt);
	m_touchEndPos = touch->getLocation();
	CCLog("touch end pos (%.1f,%.1f)", m_touchEndPos.x, m_touchEndPos.y);

	//判断移动
	m_heroMap[m_heroId]->AcceptTouchesMoved(m_touchBeginPos, m_touchEndPos);
}

void GameScene::setSceneScrollPosition(float dt)
{
	if (m_heroMap.find(m_heroId) == m_heroMap.end())
		return;

	CCPoint position = m_heroMap[m_heroId]->getPosition();
	CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
	CCSize mapSizeInPixel = CCSizeMake(m_tileMap->getMapSize().width * m_tileMap->getTileSize().width, 	m_tileMap->getMapSize().height * m_tileMap->getTileSize().height);

	if (mapSizeInPixel.width > screenSize.width)
	{
		float x = position.x-screenSize.width/2.0f;
		limit(0.0f, x, mapSizeInPixel.width-screenSize.width);
		this->setPosition(ccp(-x,this->getPosition().y));
		//CCLOG("x=%.1f, layer pos:(%.1f,%.1f)", x, this->getPosition().x, this->getPosition().y);
		m_debugLabel->setPosition(ccp(x, m_debugLabel->getPosition().y));
	}

	if (mapSizeInPixel.height > screenSize.height)
	{
		float y = position.y - screenSize.height/2.0f;
		limit(0.0f, y, mapSizeInPixel.height-screenSize.height);
		this->setPosition(ccp(this->getPosition().x,-y));
		//CCLOG("y=%.1f, layer pos:(%.1f,%.1f)", y, this->getPosition().x, this->getPosition().y);
		m_debugLabel->setPosition(ccp(m_debugLabel->getPosition().x, y));
	}
}

GameScene::~GameScene()
{
	this->unscheduleAllSelectors();
}

bool GameScene::CheckMovable( CCPoint position )
{
	if (!m_tileMap->checkMoveable(position))
		return false;

	if (!this->collisionWithBombs(position))
		return false;

	return true;
}

CCPoint GameScene::convertCoordGLToTile(const cocos2d::CCPoint& glpoint)
{
	return m_tileMap->convertCoordGLToTile(glpoint);
}

CCPoint GameScene::convertTileToCoordGL(const cocos2d::CCPoint& tilePoint)
{
	return m_tileMap->convertTileToCoordGL(tilePoint);
}

//布雷操作
bool GameScene::ToMineBomb( const CCPoint& position )
{
	if (this->IsRobotMap()) {
		this->DoMineBomb(position);
		return true;
	} else {
		CCPoint pos = convertCoordGLToTile(position);
		WorldPacket* pkg = new WorldPacket(CMSG_BOMB_BORN, 6);
		*pkg << (uint16)pos.x << (uint16)pos.y;
		Connection* connectInst = GameMediator::getInstance()->m_connectInst;
		if (connectInst) {
			connectInst->SendToServer(pkg);
			return true;
		} else {
			CCLog("connection not found");
			return false;
		}
	}
}

//埋雷
bool GameScene::DoMineBomb( const CCPoint& position )
{
	Bomb *bomb = Bomb::CreateBomb(!this->IsRobotMap());
	bomb->setPosition(position);
	bombs.push_back(bomb);
	addChild(bomb,2);
	return true;
}

//server通知client播放炸弹爆炸的动画
void GameScene::MineExplode(const cocos2d::CCPoint& position, uint8 radius)
{
	ASSERT(!this->IsRobotMap());
	Bomb* bomb = NULL;
	for ( std::vector<Bomb*>::iterator it = bombs.begin(); it != bombs.end(); it++) {
		if ((*it)->getPosition().equals(position)) {
			bomb = *it;
			break;
		}
	}
	bomb->_DeadOnCallback(true);
}

//server通知client播放角色死亡动画
void GameScene::RoleDead(uint16 roleId)
{
	ASSERT(!this->IsRobotMap());
	HeroMap::iterator it = m_heroMap.find(roleId);
	if (it != m_heroMap.end()) {
		it->second->setDead();
	}
}

//server通知client切到游戏结束的场景
void GameScene::GameOver(uint8 result)
{
	CCScene* scene = GameOverScene::scene(result);
	if (scene) {
		if (GameMediator::getInstance()->m_connectInst) {
			GameMediator::getInstance()->m_connectInst->StopNetwork();
			delete GameMediator::getInstance()->m_connectInst;
			GameMediator::getInstance()->m_connectInst = NULL;
		}
		GameMediator::getInstance()->m_gameScene->cleanup();
		GameMediator::getInstance()->m_gameScene = NULL;
		CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(1.2f, scene));
	}
}

bool GameScene::deleteNode( cocos2d::CCNode *node )
{
	CCLog("delete node %s",typeid(*node).name());
	if (strstr(typeid(*node).name(),"Hero"))
	{
		HeroMap::iterator it = m_heroMap.find(((Hero*)node)->getHeroId());
		if (it != m_heroMap.end()) {
			m_heroMap.erase(it);
		}
	} else if (strstr(typeid(*node).name(),"Bomb")) {
		vector<Bomb*>::iterator bombItr = find(bombs.begin(),bombs.end(),(Bomb*)node);
		if (bombItr != bombs.end())
			bombs.erase(bombItr);
	} else if (strstr(typeid(*node).name(),"Monster")) {
		vector<Monster*>::iterator monsterItr = find(monsters.begin(),monsters.end(),(Monster*)node);
		if (monsterItr != monsters.end()) {
			monsters.erase(monsterItr);
		}
	}
	removeChild(node,true);
	return true;
}

Hero* GameScene::FetchHeroById(uint16 id)
{
	HeroMap::iterator it = m_heroMap.find(id);
	if (it == m_heroMap.end())
		return NULL;
	else
		return it->second;
}

bool GameScene::collisionWithBombs( CCPoint position )
{
	CCPoint tilePosition=m_tileMap->convertCoordGLToTile(position);

	for(int i=0;i<bombs.size();i++)
	{
		CCPoint bombPostion=m_tileMap->convertCoordGLToTile(bombs[i]->getPosition());
		if (tilePosition.equals(bombPostion))
			return false;
	}
	return true;
}

bool GameScene::CanBomb( const CCPoint& pos )
{
	CCPoint tilePos = m_tileMap->convertCoordGLToTile(pos);
	return (m_tileMap->layerNamed("Tiles1")->tileGIDAt(tilePos) != 1);

}

void GameScene::CheckExplosion( cocos2d::CCPoint position)
{
	CCRect explosionRect[5];
	explosionRect[0]=CCRectMake(position.x,position.y,TILE_SIZE-1,TILE_SIZE-1);
	explosionRect[1]=CCRectMake(position.x-TILE_SIZE,position.y,TILE_SIZE-1,TILE_SIZE-1);
	explosionRect[2]=CCRectMake(position.x+TILE_SIZE,position.y,TILE_SIZE-1,TILE_SIZE-1);
	explosionRect[3]=CCRectMake(position.x,position.y-TILE_SIZE,TILE_SIZE-1,TILE_SIZE-1);
	explosionRect[4]=CCRectMake(position.x,position.y+TILE_SIZE,TILE_SIZE-1,TILE_SIZE-1);

	for (int i=0; i<5; i++)
	{
		//炸毁墙壁
		m_tileMap->removeWall(ccp(explosionRect[i].origin.x, explosionRect[i].origin.y));
		if (this->IsRobotMap()) {
			//炸死monster
			for(int j=0; j<monsters.size(); j++)
			{
				if (!monsters[j]->isDead()&&monsters[j]->getRect().intersectsRect(explosionRect[i]))
					monsters[j]->setDead();
			}
			//炸死hero
			for (HeroMap::iterator it = m_heroMap.begin(); it != m_heroMap.end(); it++) {
				Hero* hero = it->second;
				if (hero->getRect().intersectsRect(explosionRect[i]))
					hero->setDead();
			}
		}
	}
}

void GameScene::UpdateDebugInfo(const char* str)
{
	m_debugLabel->setString(str);
}

void GameScene::CheckGameOver(float dt)
{
	ASSERT(this->IsRobotMap());
	bool gameover = false;
	bool win;
	HeroMap::iterator it = m_heroMap.find(m_heroId);
	if (it == m_heroMap.end()) { //lose
		gameover = true;
		win = false;
	} else if (monsters.empty() && m_heroMap.size() == 1) { //win
		ASSERT(m_heroMap.begin()->first == m_heroId);
		gameover = true;
		win = true;
	}
	if (gameover)
		this->GameOver(int(win));
}

void GameScene::CheckHeroDead(float dt)
{
	for(int i=0;i<monsters.size();i++)
	{
		Monster* monster = monsters[i];
		CCRect monsterRect = monster->getRect();
		for (HeroMap::iterator it = m_heroMap.begin(); it != m_heroMap.end(); it++){
			Hero* hero = it->second;
			CCRect heroRect = hero->getRect();
			if (!monsters[i]->isDead()&&!hero->isDead()&&monsterRect.intersectsRect(heroRect))
			{
				hero->setDead();
			}
		}
	}
}

void GameScene::ChangeConnectStatus(CONNECT_STATUS status)
{
	switch(status)
	{
	case CONNECT_LOST:
		{
			this->GameOver(2);
			break;
		}
	default:
		{
			ASSERT(false);
			break;
		}
	}
}