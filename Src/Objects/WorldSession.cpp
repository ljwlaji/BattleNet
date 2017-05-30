#include "WorldSession.h"
#include "WorldPacket.h"
#include "Opcodes.h"
//int Worldsession::SendPacket(const WorldPacket* packet)
//{
//	char send_buffer[1024];
//	int send_len = 8 + packet->size();
//	*((int*)send_buffer) = send_len;
//	*((int*)(send_buffer + 4)) = packet->GetOpcode();
//	memcpy(send_buffer + 8, packet->contents(), packet->size());
//
//	//int ret = send(m_Socket, send_buffer, send_len, 0);
//	return 0;
//}

WorldSession::WorldSession(const uint32& _Socket) : m_Socket(_Socket), m_Player(nullptr)
{
}

WorldSession::~WorldSession()
{
}

void WorldSession::Update(const uint32& diff)
{
	WorldPacket* packet = nullptr;
	uint32 packetsThisCycle = 0;

	while (m_Socket && ++packetsThisCycle <= 20)
	{
		WorldPacket* packet = GetNextPacket();
		if (!packet)
			continue;
		OpcodeHandler const& opHandle = opcodeTable[packet->GetOpcode()];
		ExecuteOpcode(opHandle, packet);

		delete packet;
	}
}

void WorldSession::Handle_NULL(WorldPacket & recvPacket)
{

}

void WorldSession::PushPacket(const char * Packet)
{
	ThreadLocker loc(Packet_Lock);
	m_Packet_Queue.push_back(Packet);
}

void WorldSession::PopPacket()
{
	ThreadLocker loc(Packet_Lock);
	m_Packet_Queue.pop_front();
}

WorldPacket* WorldSession::GetNextPacket()
{
	if (m_Packet_Queue.empty())
		return nullptr;

	std::list<const char*>::iterator itr = m_Packet_Queue.begin();
	uint16 Lenth = *((uint8*)*itr) - 8;
	uint16 Opcode = *((uint8*)(*itr + 4));
	WorldPacket * pck = new WorldPacket(Opcode, Lenth);
	pck->resize(Lenth);
	pck->put(0, (const unsigned char*)*itr + 8, Lenth);
	PopPacket();
	return pck;
}


void WorldSession::ExecuteOpcode(OpcodeHandler const& opHandle, WorldPacket* packet)
{
	// need prevent do internal far teleports in handlers because some handlers do lot steps
	// or call code that can do far teleports in some conditions unexpectedly for generic way work code
	if (m_Player)
	{
		//_player->SetCanDelayTeleport(true);
	}

	(this->*opHandle.handler)(*packet);
	//if (_player)
	//{
	//	// can be not set in fact for login opcode, but this not create porblems.
	//	_player->SetCanDelayTeleport(false);
	//
	//	//we should execute delayed teleports only for alive(!) players
	//	//because we don't want player's ghost teleported from graveyard
	//	if (_player->IsHasDelayedTeleport())
	//		_player->TeleportTo(_player->m_teleport_dest, _player->m_teleport_options);
	//}
}