#ifndef __CHAT_H__
#define __CHAT_H__
#include "Common.h"

enum BattleNet_Message_Channel
{
	Whisper_Channel,
	World_Channel,
	Guild_Channel,
};

enum BattleNet_Message_Status
{
	Status_Wait,
	Status_Success,
	Status_UnSuccess,
	Status_PlayerOffLine,
};

#define sChat Chat::GetInstance()

class Chat
{
public:
	static Chat* GetInstance();
	void OnStart();
	void Update(const uint32& diff);
private:
	Chat();
	~Chat();
	void HandleChatOutPut();

	uint32 ChatResultDelay;
	uint32 CurrentDelay;
	Result _res;
	std::vector<RowInfo> SingleRow;
	std::string	SenderName;
	BattleNet_Message_Channel Channel;
	uint32 recv_battle_net_account;
	std::string message;
	uint32 date;
	BattleNet_Message_Status Status;
	uint32 guild_id;
};


#endif