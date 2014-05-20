#include "GameOverScene.h"
#include "TitleScene.h"
#include "tools.h"

using namespace cocos2d;


bool GameOverScene::init()
{
	if ( !CCLayer::init() )
	{
		return false;
	}
	return true;
}

void GameOverScene::_init(int result)
{
	bool bRet = false;
	do 
	{
		CCSize size = CCDirector::sharedDirector()->getWinSize();

		std::string titleStr;
		if (result == 1)
			titleStr = "赢了赢了！哈哈哈哈！";
		else if (result == 0)
			titleStr = "不幸败北！既生瑜何生亮！";
		else if (result == 2)
			titleStr = "擦，服务器躲哪儿去了？？";
		else
			titleStr = "恭喜，中大招了！";

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(titleStr,"gb2312","utf-8");
#endif
		CCLabelTTF* pLabel = CCLabelTTF::create(titleStr.c_str(), "Thonburi", 30);
		CC_BREAK_IF(! pLabel);
		pLabel->setColor(ccRED);
		// Get window size and place the label upper. 		
		pLabel->setPosition(ccp(size.width / 2, size.height*0.7f));
		// Add the label to TitleScene layer as a child layer.
		this->addChild(pLabel, 1);

		std::string titleStr2 = "点击屏幕返回";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(titleStr2,"gb2312","utf-8");
#endif
		CCLabelTTF* pLabel2 = CCLabelTTF::create(titleStr2.c_str(), "Thonburi", 30);
		CC_BREAK_IF(! pLabel2);
		pLabel2->setColor(ccRED);
		// Get window size and place the label upper. 		
		pLabel2->setPosition(ccp(size.width / 2, size.height*0.5f));
		// Add the label to TitleScene layer as a child layer.
		this->addChild(pLabel2, 1);

		this->setTouchEnabled(true);

		bRet = true;
	}while (0);
}

cocos2d::CCScene* GameOverScene::scene(int result)
{
	// 'scene' is an autorelease object
	CCScene *scene = CCScene::create();

	// 'layer' is an autorelease object
	GameOverScene *layer = GameOverScene::create();
	layer->_init(result);

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void GameOverScene::ccTouchesBegan( cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent )
{
	CCScene *scene=TitleScene::scene();
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(1.2f,scene));
}

bool ConnectFailScene::init()
{
	bool bRet = false;
	do 
	{
		CCSize size = CCDirector::sharedDirector()->getWinSize();

		std::string titleStr = "连接失败";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(titleStr,"gb2312","utf-8");
#endif
		CCLabelTTF* pLabel = CCLabelTTF::create(titleStr.c_str(), "Thonburi", 30);
		CC_BREAK_IF(! pLabel);
		pLabel->setColor(ccRED);
		// Get window size and place the label upper. 		
		pLabel->setPosition(ccp(size.width / 2, size.height*0.7f));
		// Add the label to TitleScene layer as a child layer.
		this->addChild(pLabel, 1);

		std::string titleStr2 = "点击屏幕返回";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		GBKToUTF8(titleStr2,"gb2312","utf-8");
#endif
		CCLabelTTF* pLabel2 = CCLabelTTF::create(titleStr2.c_str(), "Thonburi", 30);
		CC_BREAK_IF(! pLabel2);
		pLabel2->setColor(ccRED);
		// Get window size and place the label upper. 		
		pLabel2->setPosition(ccp(size.width / 2, size.height*0.5f));
		// Add the label to TitleScene layer as a child layer.
		this->addChild(pLabel2, 1);

		this->setTouchEnabled(true);

		bRet = true;
	}while (0);

	return bRet;
}

cocos2d::CCScene* ConnectFailScene::scene(const string& errInfo)
{
	// 'scene' is an autorelease object
	CCScene *scene = CCScene::create();

	// 'layer' is an autorelease object
	ConnectFailScene* layer = ConnectFailScene::create();

	CCSize size = CCDirector::sharedDirector()->getWinSize();
	CCLabelTTF* pLabel = CCLabelTTF::create(errInfo.c_str(), "Thonburi", 30);
	pLabel->setColor(ccRED);
	// Get window size and place the label upper. 		
	pLabel->setPosition(ccp(size.width / 2, size.height*0.8f));
	// Add the label to TitleScene layer as a child layer.
	layer->addChild(pLabel, 1);

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void ConnectFailScene::ccTouchesBegan( cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent )
{
	CCScene *scene = TitleScene::scene();
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(1.2f,scene));
}
