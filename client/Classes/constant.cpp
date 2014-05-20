#include "constant.h"

AnimationFormation heroAnimation[9]=
{
	{"man",1,STAND,UP},
	{"man",1,STAND,DOWN},
	{"man",1,STAND,LEFT},
	{"man",1,STAND,RIGHT},
	{"man",2,MOVE,UP},
	{"man",2,MOVE,DOWN},
	{"man",2,MOVE,LEFT},
	{"man",2,MOVE,RIGHT},
	{"man",6,DEAD,OVERLOOK},
};

AnimationFormation bombAnimation[2]=
{
	{"bomb",3,STAND,OVERLOOK	},
	{"bomb",4,DEAD,OVERLOOK},
};

AnimationFormation monsterAnimation[2]=
{
	{"monster",3,MOVE,OVERLOOK},
	{"monster",5,DEAD,OVERLOOK},
};