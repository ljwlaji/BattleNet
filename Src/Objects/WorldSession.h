/*
* This file is part of the OregonCore Project. See AUTHORS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __WORLDSESSION_H
#define __WORLDSESSION_H

#include "Common.h"

class WorldPacket;
class Player;
struct OpcodeHandler;

// Player session in the World
class WorldSession
{
public:
	WorldSession(const uint32& _Socket);
	~WorldSession();
	void Update(const uint32& diff);
	void Handle_NULL(WorldPacket& recvPacket);
	void PushPacket(const char* Packet);
	void PopPacket();
	WorldPacket* GetNextPacket();
	void ExecuteOpcode(OpcodeHandler const& opHandle, WorldPacket* packet);
private:
	std::list<const char*> m_Packet_Queue;
	mutex Packet_Lock;
	uint32 m_Socket;
	Player* m_Player;
};
#endif

