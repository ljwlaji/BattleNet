#include "DataBase.h"
#include "Chat.h"
#include "Log.h"
#include "Config.h"
#include "World.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "WorldSession.h"
static Chat* _char = nullptr;

Chat* Chat::GetInstance()
{
	if (!_char)
		_char = new Chat();
	return _char;
}

void Chat::OnStart()
{
	sLog->OutLog("聊天系统开启...");
	sLog->OutLog("删除遗留数据...");
	sDataBase->PExcude(CharacterDataBase, "DELETE * FROM playfun_message_in_put");
	ChatResultDelay = sConfig->GetIntDefault("ChatSelectTimeDelay", 500);
	CurrentDelay = ChatResultDelay;
	sLog->OutLog("聊天系统轮询时间设置为 500(毫秒)");
}

void Chat::HandleChatOutPut()
{
	_res.clear();
	sDataBase->GetResult(CharacterDataBase, _res, "SELECT sender_player_name,channel,recv_battle_net_account,message,date,status,guild_id FROM playfun_message_out_put WHRER status = 0");
	if (!_res.empty())
	{
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			SingleRow = itr->second;
			SenderName = SingleRow.at(0).GetString();
			Channel = (BattleNet_Message_Channel)SingleRow.at(1).GetInt();
			recv_battle_net_account = SingleRow.at(2).GetInt();
			message = SingleRow.at(3).GetString();
			date = SingleRow.at(4).GetInt();
			Status = (BattleNet_Message_Status)SingleRow.at(5).GetInt();
			guild_id = SingleRow.at(6).GetInt();
			if (SingleBattleNetAccount* info = sWorld->GetBattleNetInfo(recv_battle_net_account))
			{
				if (info->IsOnline)
				{
					WorldPacket packet(SMSG_MESSAGE_CHAT, 40 + 2 + 40 + 4 + 2 + 4);
					packet << (std::string)SenderName;
					packet << (uint8)Channel;
					packet << (std::string)message;
					packet << (uint32)date;
					packet << (uint8)Status;
					packet << (uint32)guild_id;
					if (WorldSession* m_session = sWorld->GetSessionBySocket(info->m_session->GetSocket()))
					{
						m_session->SendPacket(&packet);
					}
				}
			}
		}
	}
}

void Chat::Update(const uint32 & diff)
{
	if (ChatResultDelay <= diff)
	{
		HandleChatOutPut();
		ChatResultDelay = CurrentDelay;
	}
	else ChatResultDelay -= diff;
}

Chat::Chat()
{

}

Chat::~Chat()
{
}

