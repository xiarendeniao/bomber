#ifndef __GAMEOVER_SCENE_H__
#define __GAMEOVER_SCENE_H__

#include <string>
using namespace std;

#include "cocos2d.h"

#include "SimpleAudioEngine.h"

class GameOverScene : public cocos2d::CCLayer
{
public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void _init(int result);

	// there's no 'id' in cpp, so we recommand to return the exactly class pointer
	static cocos2d::CCScene* scene(int result);

	virtual void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);

	// implement the "static node()" method manually
	CREATE_FUNC(GameOverScene);
};

class ConnectFailScene : public cocos2d::CCLayer
{
public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();  

	// there's no 'id' in cpp, so we recommand to return the exactly class pointer
	static cocos2d::CCScene* scene(const string& errInfo);

	virtual void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);

	// implement the "static node()" method manually
	CREATE_FUNC(ConnectFailScene);
};

#endif  // __GAMEOVER_SCENE_H__