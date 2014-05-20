#ifndef _HELP_SCENE_H_
#define _HELP_SCENE_H_

#include "cocos2d.h"

#include "SimpleAudioEngine.h"

class HelpScene : public cocos2d::CCLayer
{
public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();  

	// there's no 'id' in cpp, so we recommand to return the exactly class pointer
	static cocos2d::CCScene* scene();

	virtual void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);

	// implement the "static node()" method manually
	CREATE_FUNC(HelpScene);
};

#endif//_HELP_SCENE_H_
