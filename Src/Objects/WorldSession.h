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

enum AuctionListStatus
{
	ListFailded,
	ListSuccess,
};
class WorldPacket;
class Player;
struct OpcodeHandler;

// Player session in the World
class WorldSession
{
public:
	WorldSession(const uint32& _Socket,const std::string& address,const uint8& m_socketpage);
	~WorldSession();
	void Update(const uint32& diff);
	void PushPacket(const char* Packet);
	void PopPacket();
	WorldPacket* GetNextPacket();
	void ExecuteOpcode(OpcodeHandler const& opHandle, WorldPacket* packet);
	uint32 GetSocket() { return m_Socket; }
	void Close() { m_Closing = true; }
	bool IsClosing() { return m_Closing; }
	int SendPacket(const WorldPacket* Packet);
	void SendHeartBeatPacket();
private:
	std::list<const char*> m_Packet_Queue;
	mutex Packet_Lock;
	uint32 m_Socket;
	Player* m_Player;
	std::string m_Address;
	bool m_Closing;
	uint8 m_SocketPage;
	uint32 BattleNetAccount;
	uint32 LastPacketTime;
	uint8 OutPingCount;
	uint32 m_GuildId;
public:
	//handler
	void Handle_NULL(WorldPacket& recvPacket);
	void HandleMessageChatOpcode(WorldPacket& RecvPacket);
	void HandleAuthLoginOpcode(WorldPacket& recvPacket);
	void HandleClientHeartBeatOpcode(WorldPacket& /*packet*/) {}
	void HandlePlayerGetDataOpcode(WorldPacket& /*recvPacket*/);
	void HandleRequireNewsDataOpcode(WorldPacket& packet);
	void HandleRequireActionDataOpcode(WorldPacket& /*packet*/);
	void HandleAuctionHouseListItemOpcode(WorldPacket& packet);
	void HandleAuctionHouseBuyItemOpcode(WorldPacket& packet);
};
#endif

