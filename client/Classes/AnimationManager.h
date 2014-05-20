#ifndef _ANIMATION_MANAGER_H_
#define _ANIMATION_MANAGER_H_

#include "cocos2d.h"
#include "Singleton.h"
#include "type.h"

class AnimationManager:public Singleton<AnimationManager>
{
public:
	bool loadAnimation(AnimationFormation *af,int count);
	cocos2d::CCAnimate* getAnimate(char *name,Behaviour behaviour,Direction direction);
	
private:
	char *getAnimationName(char *name,Behaviour behaviour,Direction direction);
};


#endif //ANIMATION_MANAGER_H_

