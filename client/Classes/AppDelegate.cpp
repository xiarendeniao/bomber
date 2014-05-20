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

	//�������д���ᵼ�³�����android����������ʱ�õ���С��2������ʾ��ͼ�쳣��
	//ֻ��ʾ�����½���Ƭ������Ƭ41*20 ÿ����Ƭ16*16���� => ��ͼ656*320���أ� С��2����Ļ��1280*720�� markbyxds 13.8.1
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	CCEGLView::sharedOpenGLView()->setDesignResolutionSize(640, 360, kResolutionNoBorder);
#endif

	//turn on display FPS
	pDirector->setDisplayStats(true);

	// set FPS. the default value is 1.0/60 if you don't call this
	pDirector->setAnimationInterval(1.0 / 60);

	//�����߳���UI�̴߳������ݵ�CCNode����
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
