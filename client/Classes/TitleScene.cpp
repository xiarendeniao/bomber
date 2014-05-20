#include "TitleScene.h"
#include "GameScene.h"
#include "RoomScene.h"
#include "tools.h"
#include "HelpScene.h"
#include "GameMediator.h"

using namespace cocos2d;

bool TitleScene::init()
{
	//////////////////////////////
	// 1. super init first
	if ( !CCLayer::init() )
	{
		return false;
	}

	CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
	CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
		"CloseNormal.png",
		"CloseSelected.png",
		this,
		menu_selector(TitleScene::menuCloseCallback));

	pCloseItem->setPosition(ccp(origin.x + visibleSize.width - pCloseItem->getContentSize().width/2 ,
		origin.y + pCloseItem->getContentSize().height/2));

	// create menu, it's an autorelease object
	CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
	pMenu->setPosition(CCPointZero);
	this->addChild(pMenu, 1);

	/////////////////////////////
	// 3. add your codes below...
	std::string titleStr = "炸弹人";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(titleStr,"gb2312","utf-8");
	#endif
#endif
	CCLabelTTF* pLabel = CCLabelTTF::create(titleStr.c_str(), "Thonburi", 30);
	if (!pLabel)
		return false;
	pLabel->setColor(ccRED);

	// Get window size and place the label upper. 
	CCSize size = CCDirector::sharedDirector()->getWinSize();
	pLabel->setPosition(ccp(size.width / 2, size.height*0.8f));

	// Add the label to TitleScene layer as a child layer.
	this->addChild(pLabel, 1);

	CCSprite* pSprite = CCSprite::create("background.png");
	if (!pSprite)
		return false;

	// Place the sprite on the center of the screen
	pSprite->setPosition(ccp(size.width/2, size.height/2));

	// Add the sprite to TitleScene layer as a child layer.
	this->addChild(pSprite, 0);

	CCMenuItemFont::setFontName("Thonburi");
	CCMenuItemFont::setFontSize(25);

	std::string menuItemStr1 = "单机游戏";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(menuItemStr1,"gb2312","utf-8");
	#endif
#endif
	CCMenuItemFont *newGame1 = CCMenuItemFont::create(menuItemStr1.c_str(),this,menu_selector(TitleScene::menuNewCallback1));
	newGame1->setColor(ccBLACK);

	std::string menuItemStr2 = "联网游戏";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(menuItemStr2,"gb2312","utf-8");
	#endif
#endif
	CCMenuItemFont *newGame2 = CCMenuItemFont::create(menuItemStr2.c_str(),this,menu_selector(TitleScene::menuNewCallback2));
	newGame2->setColor(ccBLACK);

	std::string menuItemStr = "操作说明";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(menuItemStr,"gb2312","utf-8");
	#endif
#endif
	CCMenuItemFont *helpGame=CCMenuItemFont::create(menuItemStr.c_str(),this,menu_selector(TitleScene::menuHelpCallback));
	helpGame->setColor(ccBLACK);

	menuItemStr = "退出";
#ifdef CC_TARGET_PLATFORM
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(menuItemStr,"gb2312","utf-8");
	#endif
#endif
	CCMenuItemFont *quitGame = CCMenuItemFont::create(menuItemStr.c_str(),this,menu_selector(TitleScene::menuQuitCallback));
	quitGame->setColor(ccBLACK);

	CCMenu *menu = CCMenu::create(newGame1, newGame2,helpGame,quitGame,NULL);
	menu->alignItemsVertically();

	addChild(menu,1);

	return true;
}

cocos2d::CCScene* TitleScene::scene()
{
	// 'scene' is an autorelease object
	CCScene *scene = CCScene::create();

	// 'layer' is an autorelease object
	TitleScene *layer = TitleScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void TitleScene::menuNewCallback1( CCObject* pSender )
{
	CCScene *scene = GameScene::scene(1,1);
	CCDirector::sharedDirector()->replaceScene(CCTransitionCrossFade::create(1.2f,scene));
}

void TitleScene::menuNewCallback2( CCObject* pSender )
{
	CCScene *scene = RoomScene::scene();
	CCDirector::sharedDirector()->replaceScene(CCTransitionCrossFade::create(1.2f,scene));
	Connection* connectInst = new Connection();
	GameMediator::getInstance()->m_connectInst = connectInst;
}

void TitleScene::menuQuitCallback( CCObject* pSender )
{
	// "close" menu item clicked
	CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}

void TitleScene::menuHelpCallback( CCObject* pSender )
{
	CCScene *scene = HelpScene::scene();
	CCDirector::sharedDirector()->replaceScene(CCTransitionFlipX::create(1.2f,scene));
}

void TitleScene::menuCloseCallback(CCObject* pSender)
{
	// "close" menu item clicked
	CCDirector::sharedDirector()->end();
}