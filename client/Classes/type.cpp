#include "type.h"

char* getBehaviour( Behaviour behaviour )
{
	switch(behaviour)
	{
	case STAND:
		return "stand";
	case MOVE:
		return "move";
	case DEAD:
		return "dead";
	}
}

char* getDirection( Direction direction )
{
	switch(direction)
	{
	case UP:
		return "up";
	case DOWN:
		return "down";
	case LEFT:
		return "left";
	case RIGHT:
		return "right";
	case OVERLOOK:
		return "";
	}
}
