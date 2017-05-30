#ifndef __WORLD_UPDATER_H__
#define __WORLD_UPDATER_H__
#include "Common.h"
class WorldUpdater
{
public:
	static void StartUpdate();
private:
	WorldUpdater();
	~WorldUpdater();
	void _update();
	uint32 TimeOld;
	uint32 TimeNew;
};

#endif