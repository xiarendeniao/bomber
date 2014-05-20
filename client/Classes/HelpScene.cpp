#include "TitleScene.h"
#include "tools.h"
#include "HelpScene.h"

using namespace cocos2d;

bool HelpScene::init()
{
	//////////////////////////////
	// 1. super init first
	if ( !CCLayer::init() )
	{
		return false;
	}

	CCSize size = CCDirector::sharedDirector()->getWinSize();

	std::string titleStr = "操作说明";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	GBKToUTF8(titleStr,"gb2312","utf-8");
#endif
	CCLabelTTF* pLabel = CCLabelTTF::create(titleStr.c_str(), "Thonburi", 30);
	if (!pLabel)
		return false;
	pLabel->setColor(ccRED);
	// Get window size and place the label upper. 		
	pLabel->setPosition(ccp(size.width / 2, size.height*0.9f));
	// Add the label to TitleScene layer as a child layer.
	this->addChild(pLabel, 1);

	std::string titleStr3 = "拖动行走，双击埋雷，杀光过关，哦也";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	GBKToUTF8(titleStr3,"gb2312","utf-8");
#endif
	CCLabelTTF* pLabel3 = CCLabelTTF::create(titleStr3.c_str(), "Thonburi", 20);
	if (!pLabel3)
		return false;
	pLabel3->setColor(ccRED);
	// Get window size and place the label upper. 		
	pLabel3->setPosition(ccp(size.width / 2, size.height*0.6f));
	// Add the label to TitleScene layer as a child layer.
	this->addChild(pLabel3, 1);

	std::string titleStr2 = "点击屏幕返回";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	GBKToUTF8(titleStr2,"gb2312","utf-8");
#endif
	CCLabelTTF* pLabel2 = CCLabelTTF::create(titleStr2.c_str(), "Thonburi", 30);
	if (!pLabel2)
		return false;
	pLabel2->setColor(ccRED);
	// Get window size and place the label upper. 		
	pLabel2->setPosition(ccp(size.width / 2, size.height*0.3f));
	// Add the label to TitleScene layer as a child layer.
	this->addChild(pLabel2, 1);

	this->setTouchEnabled(true);
	
	return true;
}

cocos2d::CCScene* HelpScene::scene()
{
	// 'scene' is an autorelease object
	CCScene *scene = CCScene::create();

	// 'layer' is an autorelease object
	HelpScene *layer = HelpScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void HelpScene::ccTouchesBegan( cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent )
{
	CCScene *scene = TitleScene::scene();
	CCDirector::sharedDirector()->replaceScene(CCTransitionFlipX::create(1.2f,scene));
}
