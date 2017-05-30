#include <Windows.h>
#include <iostream>
#include "WorldUpdater.h"
#include "World.h"
#include "Log.h"
mutex WU_Locker;
static WorldUpdater* _WorldUpdater = nullptr;

void WorldUpdater::StartUpdate()
{
	ThreadLocker loc(WU_Locker);
	if (!_WorldUpdater)
		_WorldUpdater = new WorldUpdater();
	_WorldUpdater->_update();

}

WorldUpdater::WorldUpdater()
{
}

WorldUpdater::~WorldUpdater()
{
}

void WorldUpdater::_update()
{
	while (1)
	{
		TimeNew = GetTickCount();
		uint32 diff = TimeNew - TimeOld;
		if (diff)
		{
			sWorld->update(diff);
		}
		TimeOld = TimeNew;
		Sleep(1);
	}
}