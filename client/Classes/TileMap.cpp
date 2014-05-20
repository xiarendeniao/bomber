#include "TileMap.h"

using namespace cocos2d;

TileMap::TileMap(void)
{
}

TileMap::~TileMap(void)
{
}

TileMap* TileMap::initTileMap( const char *tmxFile )
{
	TileMap *tileMap=new TileMap;

	if (tileMap->initWithTMXFile(tmxFile))
	{
		tileMap->autorelease();
		return tileMap;
	}

	CC_SAFE_DELETE(tileMap);
	return NULL;
}

cocos2d::CCPoint TileMap::convertCoordGLToTile( cocos2d::CCPoint glpoint )
{
	int x = glpoint.x / this->getTileSize().width;
	int y = (((this->getMapSize().height - 1) * this->getTileSize().height) - glpoint.y) / this->getTileSize().height;
	cocos2d::CCPoint newPos = this->convertTileToCoordGL(ccp(x,y));
	return ccp(x, y);
}

cocos2d::CCPoint TileMap::convertTileToCoordGL(cocos2d::CCPoint tilepoint)
{
	return ccp(tilepoint.x*this->getTileSize().width, this->getMapSize().height*this->getTileSize().height-(tilepoint.y+1)*this->getTileSize().height);
}

bool TileMap::checkMoveable( cocos2d::CCPoint glpoint )
{
	CCPoint tilePoint=convertCoordGLToTile(glpoint);

	unsigned int gid = this->layerNamed("Tiles1")->tileGIDAt(tilePoint);
	//CCLog("pos (%.2f,%.2f) tile gid %d", tilePoint.x, tilePoint.y, gid);

	return !gid;
}

bool TileMap::removeWall( cocos2d::CCPoint glpoint )
{
	CCPoint tilePoint=convertCoordGLToTile(glpoint);

	if (this->layerNamed("Tiles1")->tileGIDAt(tilePoint)==2) //2±íÊ¾Ç½
	{
		this->layerNamed("Tiles1")->removeTileAt(tilePoint);
		return true;
	}

	return false;
}
