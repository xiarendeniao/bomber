#include "GameMediator.h"
#include <typeinfo>
#include <algorithm>
#include "tools.h"
#include "constant.h"

using namespace cocos2d;
using namespace std;

GameMediator::GameMediator()
{
	m_connectInst = NULL;
	m_gameScene = NULL;
	m_roomScene = NULL;
}
