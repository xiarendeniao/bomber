#include "AppDelegate.h"

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;


#include "TitleScene.h"
#include "GameScene.h"
#include "Connection.h"

using namespace cocos2d;

AppDelegate::AppDelegate()
{

}

AppDelegate::~AppDelegate()
{
    SimpleAudioEngine::end();
}

bool AppDelegate::applicationDidFinishLaunching()
{
	// initialize director
	CCDirector *pDirector = CCDirector::sharedDirector();

	pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());

	//不加这行代码会导致程序在android机器（测试时用的是小米2）上显示地图异常：
	//只显示在左下角那片区域（瓦片41*20 每个瓦片16*16像素 => 地图656*320像素； 小米2的屏幕是1280*720） markbyxds 13.8.1
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	CCEGLView::sharedOpenGLView()->setDesignResolutionSize(640, 360, kResolutionNoBorder);
#endif

	//turn on display FPS
	pDirector->setDisplayStats(true);

	// set FPS. the default value is 1.0/60 if you don't call this
	pDirector->setAnimationInterval(1.0 / 60);

	//网络线程往UI线程传递数据的CCNode对象
	PublicMsgHandler::getInstance();

	// create a scene. it's an autorelease object
	CCScene *pScene = TitleScene::scene();

	// run
	pDirector->runWithScene(pScene);

	return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
	CCDirector::sharedDirector()->stopAnimation();

	// if you use SimpleAudioEngine, it must be pause
	SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
	CCDirector::sharedDirector()->startAnimation();

	// if you use SimpleAudioEngine, it must resume here
	SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
}
