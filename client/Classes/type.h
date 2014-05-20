#ifndef _TYPE_H_
#define _TYPE_H_

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	OVERLOOK //任何朝向都一样
}  Direction;

char* getDirection(Direction direction);

typedef enum{
	STAND,
	MOVE,
	DEAD
} Behaviour;

char* getBehaviour(Behaviour behaviour);

struct  AnimationFormation
{
	char *animateName;//动画名：对应文件开头
	int frameNum;//帧数
	Behaviour behaviour;//行为
	Direction direction;//朝向
};

#endif //_TYPE_H_