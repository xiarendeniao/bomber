#include "AnimationManager.h"
#include "constant.h"

using namespace std;
using namespace cocos2d;

static char charBuffer[128];

bool AnimationManager::loadAnimation(AnimationFormation *af,int count)
{
	//���塪�������ض�Ӧ��png�������г�SpriteFrame�����һ����������
	memset(charBuffer,0,sizeof(charBuffer));
	sprintf(charBuffer,"%s.plist",af[0].animateName);
	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(charBuffer);

	//������������
	CCArray *spriteFrames = new CCArray();

	for (int i=0;i<count;i++)
	{
		for(int j=0;j<af[i].frameNum;j++)
		{
			memset(charBuffer,0,sizeof(charBuffer));
			sprintf(charBuffer,"%s_%s%s%d.png",af[i].animateName,getBehaviour(af[i].behaviour),getDirection(af[i].direction),j);

			CCSpriteFrame *spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(charBuffer);
			spriteFrames->addObject(spriteFrame);
		}
		//ʹ��cache�������
		CCAnimation *animation = CCAnimation::createWithSpriteFrames(spriteFrames,0.3f);
		memset(charBuffer,0,sizeof(charBuffer));
		sprintf(charBuffer,"%s_%s%s",af[i].animateName,getBehaviour(af[i].behaviour),getDirection(af[i].direction));
		CCAnimationCache::sharedAnimationCache()->addAnimation(animation,charBuffer);

		spriteFrames->removeAllObjects();
	}

	spriteFrames->release();

	return true;
}

cocos2d::CCAnimate* AnimationManager::getAnimate(char *name,Behaviour behaviour,Direction direction )
{
	CCAnimation* animation=CCAnimationCache::sharedAnimationCache()->animationByName(getAnimationName(name,behaviour,direction));

	if(animation)
	{
		return cocos2d::CCAnimate::create(animation);
	}
	return NULL;
}

char* AnimationManager::getAnimationName(char *name,Behaviour behaviour,Direction direction )
{
	memset(charBuffer,0,sizeof(charBuffer));
	sprintf(charBuffer,"%s_%s%s",name,getBehaviour(behaviour),getDirection(direction));

	return charBuffer;
}
