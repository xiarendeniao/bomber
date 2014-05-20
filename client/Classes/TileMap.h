#ifndef _TILE_MAP_H_
#define _TILE_MAP_H_

#include "cocos2d.h"

class TileMap:public cocos2d::CCTMXTiledMap
{
public:
	TileMap(void);
	~TileMap(void);

	static TileMap* initTileMap(const char *tmxFile);

	cocos2d::CCPoint convertCoordGLToTile(cocos2d::CCPoint glpoint);
	cocos2d::CCPoint convertTileToCoordGL(cocos2d::CCPoint tilepoint);
	bool checkMoveable(cocos2d::CCPoint glpoint);
	bool removeWall(cocos2d::CCPoint glpoint);
};

#endif//_TILE_MAP_H_


