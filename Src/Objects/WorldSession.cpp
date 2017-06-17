#include "WorldSocket.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "DataBase.h"
#include "Chat.h"
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

WorldSession::WorldSession(const uint32& _Socket, const std::string& Address, const uint8& SockePage) : m_Socket(_Socket), m_Player(nullptr), m_Address(Address), m_Closing(false), m_SocketPage(SockePage), BattleNetAccount(0), OutPingCount(0),LastPacketTime(time(NULL)), m_GuildId(0)
{
	sLog->OutLog("Create New Session");
}

WorldSession::~WorldSession()
{
	if (SingleBattleNetAccount* AccountInfo = sWorld->GetBattleNetInfo(BattleNetAccount))
	{
		AccountInfo->IsOnline = false;
		AccountInfo->m_session = nullptr;
	}
	sWorldSocket->CloseSocket(m_Socket, m_SocketPage);
	sLog->OutLog("Delete Session");
}

void WorldSession::Update(const uint32& diff)
{
	WorldPacket* packet = nullptr;
	uint32 packetsThisCycle = 0;

	while (m_Socket && !m_Closing && ++packetsThisCycle <= 20)
	{
		WorldPacket* packet = GetNextPacket();
		if (!packet || !packet->size())
			continue;
		OpcodeHandler const& opHandle = opcodeTable[packet->GetOpcode()];
		ExecuteOpcode(opHandle, packet);
		LastPacketTime = time(NULL);
		delete packet;
	}

	if (LastPacketTime + 10 < time(NULL))
		SendHeartBeatPacket();


}

void WorldSession::Handle_NULL(WorldPacket & recvPacket)
{

}

void WorldSession::HandleMessageChatOpcode(WorldPacket & RecvPacket)
{
	std::string SenderName = "";
	uint32 Channel = Whisper_Channel;
	std::string Message = "";
	uint32 RecvPlayer = 0;
	RecvPacket >> SenderName;
	RecvPacket >> Channel;
	RecvPacket >> Message;

	sChat->HandleMessageInPut(SenderName, (BattleNet_Message_Channel)Channel, Message, RecvPlayer, m_GuildId);
}

void WorldSession::HandlePlayerGetDataOpcode(WorldPacket& /*recvPacket*/)
{
	WorldPacket packet(SMGS_AUTH_DATA_RESULT,4 * 5 + 40);
	std::string Name = "";
	uint32 Race = 0;
	uint32 Class = 0;
	uint32 Gender = 0;
	uint32 Level = 0;
	uint32 Money = 0;
	if (SingleBattleNetAccount* AccountInfo = sWorld->GetBattleNetInfo(BattleNetAccount))
	{
		if (AccountInfo->CharacterToShow)
		{
			Result _res;//												0	1	  2		3		4	5
			if (sDataBase->GetResult(CharacterDataBase, _res, "SELECT name,race,class,gender,level,money FROM characters WHERE guid = %d", AccountInfo->CharacterToShow) && !_res.empty())
			{
				std::vector<RowInfo> ReInfo = _res.begin()->second;
				Name	= ReInfo.at(0).GetString();
				Race	= ReInfo.at(1).GetInt();
				Class	= ReInfo.at(2).GetInt();
				Gender	= ReInfo.at(3).GetInt();
				Level	= ReInfo.at(4).GetInt();
				Money	= ReInfo.at(5).GetInt();
			}
			_res.clear();
			if (sDataBase->GetResult(CharacterDataBase, _res, "SELECT guildid FROM guild_member WHERE guid = %d", AccountInfo->CharacterToShow) && !_res.empty())
			{
				m_GuildId = _res.begin()->second.at(0).GetInt();
			}
		}
	}
	packet << Name;
	packet << Race;
	packet << Class;
	packet << Gender;
	packet << Level;
	packet << Money;
	packet << m_GuildId;
	SendPacket(&packet);
}

void WorldSession::HandleRequireNewsDataOpcode(WorldPacket & RecvPacket)
{
	uint16 MessageNumber = 0;
	RecvPacket >> MessageNumber;
	WorldPacket packet(SMSG_NEWS_REQUIRE);
	uint32 Success = 0;
	if (const NewInfoTemplate* Template = sWorld->GetNewsInfo(MessageNumber))
	{
		Success = 1;
		packet << Success;
		packet << (uint32)Template->Time;
		packet << (std::string)Template->Title;
		packet << (std::string)Template->Message;
	}
	else
	{
		packet << Success;
	}

	SendPacket(&packet);
}

void WorldSession::HandleRequireActionDataOpcode(WorldPacket & /*packet*/)
{
	WorldPacket packet(SMSG_ACTIONS_REQUIRE);
	if (const std::list<std::string>* info = sWorld->GetActionInfo())
		for (std::list<std::string>::const_iterator itr = info->begin(); itr != info->end(); itr++)
			packet << (*itr);

	SendPacket(&packet);
}

void WorldSession::HandleAuctionHouseListItemOpcode(WorldPacket & packet)
{
	//for (int i = 34424; i != 40000; i++)
	//{
	//	if (const ItemTemplate* TempTemplate = sWorld->GetItemTemplate(i))
	//	{
	//		std::string url = sWorld->GetItemUrl(TempTemplate->DisPlayid).c_str();
	//		WorldPacket packet(SMSG_INSERT_ICON);
	//		packet << TempTemplate->Entry;
	//		packet << (std::string)url;
	//		SendPacket(&packet);
	//		Sleep(40);
	//	}
	//}
	uint32 TeamID = 0;
	uint32 PageID = 0;
	std::string Item = "";
	packet >> TeamID;
	packet >> PageID;
	packet >> Item;

	if (!PageID)
		return;

	std::map<uint32, AuctionItem> result;
	uint32 ItemCount = sWorld->GetAuctionItems(TeamID, PageID, result, Item);

	WorldPacket Sendpacket(SMSG_AUCTION_LIST_ITEM);
	if (!result.size())
	{
		Sendpacket << (uint32)ListFailded;
		SendPacket(&Sendpacket);
		return;
	}

	Sendpacket << (uint32)ListSuccess;
	Sendpacket << (uint32)ItemCount;

	for (std::map<uint32, AuctionItem>::iterator itr = result.begin(); itr != result.end(); itr++)
	{
		AuctionItem SingleAuctionItem = itr->second;
		Sendpacket << (uint32)SingleAuctionItem.id;
		Sendpacket << (uint32)SingleAuctionItem.item_template;
		//Result _result;
		//sDataBase->GetResult(CharacterDataBase, _result, "SELECT name FROM characters WHERE guid = %d", itr->second.item_owner);
		//if (!_result.empty())
		//	Sendpacket << (std::string)_result.begin()->second.at(0).GetString();
		//else
		//	Sendpacket << (std::string)"null";


		//Sendpacket << (uint32)SingleAuctionItem.houseid;
		//Sendpacket << (std::string)sWorld->GetItemLocalString(SingleAuctionItem.item_template)->Name_Chinese_Normal;
		Sendpacket << (uint32)SingleAuctionItem.buyout_price;
		//Sendpacket << (uint32)SingleAuctionItem.time;
		Sendpacket << (uint32)SingleAuctionItem.last_bid;
		//Sendpacket << (uint32)SingleAuctionItem.start_bid;
	}
	SendPacket(&Sendpacket);
}

void WorldSession::HandleAuctionHouseBuyItemOpcode(WorldPacket & packet)
{
	if (SingleBattleNetAccount* AccountInfo = sWorld->GetBattleNetInfo(BattleNetAccount))
	{
		if (AccountInfo->CharacterToShow)
		{
			uint32 AuctionID = 0;
			uint32 Price = 0;
			uint32 TeamID = 0;
			packet >> AuctionID;
			packet >> Price;
			packet >> TeamID;														//		0		1			2		3				4		5		
			sDataBase->PExcude(CharacterDataBase, "INSERT INTO playfun_battle_net_auction(team_id,auctionid,bidsocket,bidplayerguid,bidprice,status) VALUES(%d,%d,%d,%d,%d,%d)", TeamID, AuctionID, m_Socket, AccountInfo->CharacterToShow, Price, 0);
		}
	}
}

void WorldSession::HandleAuthLoginOpcode(WorldPacket & recvPacket)
{
	if (BattleNetAccount)
		return;
	std::string account = "";
	std::string password = "";
	recvPacket >> account;
	recvPacket >> password;
	if (account.empty() || password.empty())
	{
		sLog->OutBug("接受到来自 %s 的空登录数据包,自动断开连接", m_Address.c_str());
		Close();
		return;
	}
	//HandleResult
	Result _res;
	uint8 LoginResult = Login_Failed;
	sDataBase->GetResult(LoginDataBase, _res, "SELECT lingked_account_id FROM playfun_battle_net_accounts WHERE battle_net_username = '%s' AND password = '%s'", account.c_str(), password.c_str());
	if (!_res.empty())
	{
		if (SingleBattleNetAccount* AccountInfo = sWorld->GetBattleNetInfo(_res.begin()->second.at(0).GetInt()))
		{
			WorldSession* session = AccountInfo->m_session;
			if (AccountInfo->IsOnline)
			{
				if (session && AccountInfo->m_session->m_Socket != m_Socket)
				{
					sWorldSocket->CloseSocket(AccountInfo->m_session->m_Socket, AccountInfo->m_session->m_SocketPage);
				}
			}
			AccountInfo->IsOnline = true;
			LoginResult = Login_Success;
			BattleNetAccount = _res.begin()->second.at(0).GetInt();
			AccountInfo->m_session = this;
		}
		else
		{
			sLog->OutBug("Player Login With A Empty BattleNetAccount MayBe It's Not Loaded Yet!");
		}
	}
	WorldPacket packet(SMSG_LOGIN_RESULT, 2);
	packet << LoginResult;
	SendPacket(&packet);
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
	uint16 TotalSize = *((uint16*)*itr);
	if (TotalSize < 6 || TotalSize > 200)
	{
		sLog->OutBug("UnKnow Packet From %d", m_Socket);
		PopPacket();
		return nullptr;
	}
	uint16 PacketLenth = *((uint16*)*itr) - 6;
	uint16 Opcode = *((uint16*)(*itr + 4));
	WorldPacket * pck = new WorldPacket(Opcode, PacketLenth);
	pck->resize(PacketLenth);
	if (PacketLenth)
		pck->put(0, (const unsigned char*)*itr + 6, PacketLenth);
	else
	{
		sLog->OutBug("Socket %d Sended An Empty Packet", m_Socket);
		PopPacket();
		delete pck;
		return nullptr;
	}
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

	LastPacketTime = time(NULL);
	OutPingCount = 0;
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

int WorldSession::SendPacket(const WorldPacket* Packet)
{
	return sWorldSocket->SendPacket(m_Socket, *Packet);
}

void WorldSession::SendHeartBeatPacket()
{
	sLog->OutLog("Out Ping %d", OutPingCount);
	if (++OutPingCount > 2)
	{
		Close();
		return;
	}
	WorldPacket packet(SMSG_HEARTBEAT_RESULT, 4);
	packet << (uint8)1;
	SendPacket(&packet);
	LastPacketTime = time(NULL);
}
