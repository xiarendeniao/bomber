#ifndef _TYPE_H_
#define _TYPE_H_

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	OVERLOOK //�κγ���һ��
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
	char *animateName;//����������Ӧ�ļ���ͷ
	int frameNum;//֡��
	Behaviour behaviour;//��Ϊ
	Direction direction;//����
};

#endif //_TYPE_H_