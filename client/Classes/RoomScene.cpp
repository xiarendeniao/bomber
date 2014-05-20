#include <string.h>
#include "RoomScene.h"
#include "TitleScene.h"
#include "GameMediator.h"
#include "tools.h"

using namespace std;

cocos2d::CCScene* RoomScene::scene()
{
	CCScene *scene = CCScene::create();
	RoomScene *layer = RoomScene::create();
	scene->addChild(layer);
	return scene;
}

RoomScene::~RoomScene(void)
{
}

bool RoomScene::init()
{
	m_connectLabel = NULL;
	m_roomMenu = NULL;
	m_createMenu = NULL;
	m_leaveMenu = NULL;
	m_roomLayer = NULL;

	if ( !CCLayer::init() )
		return false;

	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	//网络提示标签
	m_connectLabel = CCLabelTTF::create("", "Arial", 20);
	m_connectLabel->setAnchorPoint(CCPointZero);
	CCSize labelSize = m_connectLabel->getContentSize();
	m_connectLabel->setPosition(CCPoint(winSize.width-labelSize.width, winSize.height-labelSize.height));
	m_connectLabel->setColor(ccGREEN);
	CCLog("connectLabel position (%.2f,%.2f)", m_connectLabel->getPosition().x, m_connectLabel->getPosition().y);
	this->addChild(m_connectLabel,1);

 	//返回按钮
	string returnStr = "返回";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(returnStr,"gb2312","utf-8");
	#endif
#endif
	CCMenuItemFont *returnItem = CCMenuItemFont::create(returnStr.c_str(), this, menu_selector(RoomScene::MenuReturnCallback));
	returnItem->setColor(ccBLUE);
	CCSize returnMenuSize = returnItem->getContentSize();
	returnItem->setAnchorPoint(ccp(0,0));
	returnItem->setPosition(ccp(winSize.width - returnMenuSize.width, 0));
	CCMenu* menu = CCMenu::create();
	menu->addChild(returnItem);
	menu->setPosition(CCPointZero);
	this->addChild(menu, 1);

	//创建房间的按钮
	string newStr = "新建房间";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(newStr,"gb2312","utf-8");
	#endif
#endif
	CCMenuItemFont* newRoomItem = CCMenuItemFont::create(newStr.c_str(), this, menu_selector(RoomScene::MenuNewRoomCallback));
	newRoomItem->setColor(ccBLUE);
	CCSize newRoomMenuSize = newRoomItem->getContentSize();
	newRoomItem->setAnchorPoint(ccp(0,0));
 	newRoomItem->setPosition(ccp(winSize.width - returnMenuSize.width - newRoomMenuSize.width - 5, 0)); 
 	m_createMenu = CCMenu::create();
 	m_createMenu->addChild(newRoomItem);
	m_createMenu->setPosition(CCPointZero);
 	this->addChild(m_createMenu);
 	m_createMenu->setEnabled(false);
 	m_createMenu->setColor(ccGRAY);

	//退出房间的按钮
	string leaveStr = "离开房间";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(leaveStr,"gb2312","utf-8");
	#endif
#endif
	CCMenuItemFont* leaveItem = CCMenuItemFont::create(leaveStr.c_str(), this, menu_selector(RoomScene::MenuLeaveRoomCallback));
	leaveItem->setColor(ccBLUE);
	CCSize leaveMenuSize = leaveItem->getContentSize();
	leaveItem->setAnchorPoint(ccp(0,0));
	leaveItem->setPosition(ccp(winSize.width - returnMenuSize.width - newRoomMenuSize.width - leaveMenuSize.width - 10, 0));
 	m_leaveMenu = CCMenu::create();
	m_leaveMenu->addChild(leaveItem);
	m_leaveMenu->setPosition(CCPointZero);
	this->addChild(m_leaveMenu);
	m_leaveMenu->setEnabled(false);
	m_leaveMenu->setColor(ccGRAY);

	//房间列表
	m_roomMenu = CCMenu::create();
	m_roomMenu->setPosition(CCPointZero);
	this->addChild(m_roomMenu, 0);

	//开启触摸
	this->setTouchEnabled(true);

	//加入资源管理器
	GameMediator::getInstance()->m_roomScene = this;

	return true;
}

void RoomScene::MenuReturnCallback(CCObject* pSender)
{
	CCScene *scene = TitleScene::scene();
	CCDirector::sharedDirector()->replaceScene(CCTransitionShrinkGrow::create(1.2f,scene));
	GameMediator::getInstance()->m_roomScene->cleanup();
	GameMediator::getInstance()->m_roomScene = NULL;
 	if (GameMediator::getInstance()->m_connectInst) {
 		GameMediator::getInstance()->m_connectInst->StopNetwork();
 		delete GameMediator::getInstance()->m_connectInst;
 		GameMediator::getInstance()->m_connectInst = NULL;
 	}
}

void RoomScene::MenuNewRoomCallback(CCObject* pSender)
{
	uint16 mapId = 2001;
	uint8 maxCount = 2;
	WorldPacket* pkg = new WorldPacket(CMSG_ROOM_CREATE, 6);
	*pkg << (uint16)mapId << (uint8)maxCount;
 	Connection* connectInst = GameMediator::getInstance()->m_connectInst;
 	if (connectInst)
 		connectInst->SendToServer(pkg);
 	else
		CCLog("connection not found");
}

void RoomScene::MenuLeaveRoomCallback(CCObject* pSender)
{
	if (m_roomLayer) {
		WorldPacket* pkg = new WorldPacket(CMSG_ROOM_LEAVE, 2);
		*pkg << (uint16)m_roomLayer->m_roomId;
		Connection* connectInst = GameMediator::getInstance()->m_connectInst;
		if (connectInst)
			connectInst->SendToServer(pkg);
		else
			CCLog("connection not found");
	}
}

void RoomScene::MenuRoomCallback(CCObject* pSender)
{
	CCMenuItemFont* menuItem = (CCMenuItemFont*)pSender;
	uint16 roomId = (uint16)menuItem->getZOrder();
	WorldPacket* pkg = new WorldPacket(CMSG_ROOM_ENTER, 20);
	*pkg << (uint16)roomId;
	Connection* connectInst = GameMediator::getInstance()->m_connectInst;
	if (connectInst)
		connectInst->SendToServer(pkg);
	else
		CCLog("connection not found");
}

void RoomScene::UpdateRoomList(WorldPacket* msg)
{
	m_roomMenu->removeAllChildrenWithCleanup(true);
	const uint16 ITEM_HEIGHT = 20;
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	char buff[24];
	uint16 roomSum, roomId;
	uint8 maxCount, enterCount;
	*msg >> roomSum;
	for (uint8 i = 0; i < roomSum; i++) {
		*msg >> roomId >> maxCount >> enterCount;
		sprintf(buff, "room-%d(%d/%d)", roomId, enterCount, maxCount);
		CCMenuItemFont* menuItem = CCMenuItemFont::create(buff, this, menu_selector(RoomScene::MenuRoomCallback));
		menuItem->setTag(roomId);
		if (m_roomLayer && roomId == m_roomLayer->m_roomId)
			menuItem->setColor(ccORANGE);
		else
			menuItem->setColor(ccBLUE);
		menuItem->setAnchorPoint(CCPointZero);
		menuItem->setPosition(ccp(0,winSize.height-(i+1)*ITEM_HEIGHT));
		m_roomMenu->addChild(menuItem, (int)roomId);
	}
	m_roomMenu->setContentSize(CCSizeMake(winSize.width/3, ITEM_HEIGHT*roomSum));
}

void RoomScene::LeaveRoom(WorldPacket* pkg)
{
	uint16 oldRoomId = 0;
	if (m_roomLayer) {
		oldRoomId = m_roomLayer->m_roomId;
		m_roomLayer->removeFromParentAndCleanup(true);
		m_roomLayer = NULL;
		m_leaveMenu->setEnabled(false);
		m_leaveMenu->setColor(ccGRAY);
	}
	//处理房间列表中对应条目的颜色
	if (oldRoomId) {
		CCMenuItemFont* item = (CCMenuItemFont*)(m_roomMenu->getChildByTag(oldRoomId));
		if (item)
			item->setColor(ccBLUE);
	}
}

void RoomScene::UpdateRoomInfo(WorldPacket* pkg)
{
	uint16 oldRoomId = 0;
	if (m_roomLayer) {
		oldRoomId = m_roomLayer->m_roomId;
		this->removeChild(m_roomLayer, true);
		m_roomLayer = NULL;
	}
	
	m_roomLayer = new RoomLayer(pkg, this);
	m_roomLayer->release();
	m_leaveMenu->setEnabled(true);
	m_leaveMenu->setColor(ccBLUE);

	//处理房间列表中对应条目的颜色
	if (oldRoomId) {
		CCMenuItemFont* item = (CCMenuItemFont*)(m_roomMenu->getChildByTag(oldRoomId));
		if (item)
			item->setColor(ccBLUE);
	}
	CCMenuItemFont* item = (CCMenuItemFont*)(m_roomMenu->getChildByTag(m_roomLayer->m_roomId));
	if (item)
		item->setColor(ccORANGE);
}

void RoomScene::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)
{
	CCSetIterator it = pTouches->begin();
	CCTouch* touch = (CCTouch*)(*it);
	m_beginPos = touch->getLocation();    
}

void RoomScene::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)
{
	if (!m_roomMenu)
		return;

	CCSetIterator it = pTouches->begin();
	CCTouch* touch = (CCTouch*)(*it);
	CCPoint touchLocation = touch->getLocation();    
	float moveY = touchLocation.y - m_beginPos.y;

	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	CCSize menuSize = m_roomMenu->getContentSize();
	if (menuSize.height < winSize.height)
		return;

	CCPoint curPos = m_roomMenu->getPosition();
	CCPoint nextPos = ccp(curPos.x, curPos.y + moveY);
	if (nextPos.y < 0.0f) {
		m_roomMenu->setPosition(CCPointZero);
		return;
	}
	if (nextPos.y > menuSize.height - winSize.height) {
		m_roomMenu->setPosition(ccp(0, menuSize.height - winSize.height));
		return;
	}

	m_roomMenu->setPosition(nextPos);
	m_beginPos = touchLocation;
}

void RoomScene::ChangeConnectStatus(CONNECT_STATUS status)
{
	switch(status)
	{
	case CONNECT_ING:
		{
			m_connectLabel->setString("connecting to server");
			break;
		}
	case CONNECT_SUCCESS:
		{
			m_connectLabel->setString("connect to server success");
			m_createMenu->setEnabled(true);
			m_createMenu->setColor(ccBLUE);
			break;
		}
	case CONNECT_FAILED:
		{
			m_connectLabel->setString("connect to server failed");
			break;
		}
	case CONNECT_LOST:
		{
			m_connectLabel->setString("connection lost");
			m_roomLayer->removeFromParentAndCleanup(true);
			m_roomLayer = NULL;
			m_roomMenu->removeFromParentAndCleanup(true);
			m_roomMenu = NULL;
			m_createMenu->setEnabled(false);
			m_createMenu->setColor(ccGRAY);
			m_leaveMenu->setEnabled(false);
			m_leaveMenu->setColor(ccGRAY);
			break;
		}
	default:
		{
			char buff[50] = {0};
			sprintf(buff, "unrecognized status %d", status);
			m_connectLabel->setString(buff);
			break;
		}
	}
	CCSize labelSize = m_connectLabel->getContentSize();
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	float x = winSize.width - labelSize.width;
	if (x < winSize.width/3.0)
		x = winSize.width/3.0;
	m_connectLabel->setPosition(CCPoint(x, winSize.height - 20));
}

RoomLayer::RoomLayer(WorldPacket* pkg, RoomScene* roomScene)
{
	m_bodyMenu = NULL;
	m_roomScene = NULL;

	//解析SMSG_ROOM_INFO数据包并构造RoomMenu
	uint16 roomId, mapId;
	uint8 maxCount, enterCount, ready;
	uint32 accountId;
	string accountName;
	*pkg >> roomId >> mapId >> maxCount >> enterCount;

	m_roomId = roomId;
	m_mapId = mapId;
	m_maxCount = maxCount;
	m_enterCount = enterCount;

	m_bodyMenu = CCMenu::create();
	char buff[512];
	const uint16 ITEM_HEIGHT = 20;
	for (uint8 i = 0; i < enterCount; i++) {
		*pkg >> accountId >> accountName >> ready;
		sprintf(buff, "%d(%s)", accountId, accountName.c_str());
		CCMenuItemFont* bodyItem = CCMenuItemFont::create(buff, this, menu_selector(RoomLayer::MenuBodyCallback));
		if (ready == 1)
			bodyItem->setColor(ccORANGE);
		else
			bodyItem->setColor(ccGRAY);
		bodyItem->setAnchorPoint(CCPointZero);
		bodyItem->setPosition(ccp(0,-(i+1)*ITEM_HEIGHT));
		m_bodyMenu->addChild(bodyItem, (int)accountId);
	}
	this->addChild(m_bodyMenu);

	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	m_bodyMenu->setPosition(winSize.width/3.0, winSize.height-20);
	m_roomScene = roomScene;
	roomScene->addChild(this);
}

RoomLayer::~RoomLayer()
{
	CCLog("RoomLayer %d destructed...", m_roomId);
}

void RoomLayer::MenuBodyCallback(CCObject* pSender)
{
	CCMenuItemFont* menuItem = (CCMenuItemFont*)pSender;
	int accountId = menuItem->getZOrder();
	uint16 tmp = (uint16)accountId;
	CCLog("body %d clicked", accountId);
}