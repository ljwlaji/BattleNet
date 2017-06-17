#ifndef _OPCODES_H
#define _OPCODES_H

#include "Common.h"
#include "WorldSession.h"

// List of Opcodes
enum Opcodes
{
	MSG_NULLPTR				= 0x000,
	CMSG_HEARTBEAT_RESULT	= 0x001,
	SMSG_HEARTBEAT_RESULT	= 0x002,
	CMSG_LOGIN_REQUIRE		= 0x003,
	SMSG_LOGIN_RESULT		= 0x004,
	CMSG_AUTH_DATA_RESULT	= 0x005,
	SMGS_AUTH_DATA_RESULT	= 0X006,
	CMSG_MESSAGE_CHAT		= 0x007,
	SMSG_MESSAGE_CHAT		= 0x008,
	CMSG_NEWS_REQUIRE		= 0x009,
	SMSG_NEWS_REQUIRE		= 0x00A,
	CMSG_ACTIONS_REQUIRE	= 0x00B,
	SMSG_ACTIONS_REQUIRE	= 0x00C,
	CMSG_AUCTION_LIST_ITEM	= 0x00D,
	SMSG_AUCTION_LIST_ITEM	= 0x00E,
	CMSG_AUCTION_BUY_ITEM	= 0x00F,
	SMSG_AUCTION_BUY_ITEM	= 0X010,
	//SMSG_INSERT_ICON		= 0x00F,
};

enum LoginResult
{
	Login_Success,
	Login_Failed,
};


#define NUM_MSG_TYPES 0x424

/// Player state
enum SessionStatus
{
	STATUS_AUTHED			= 0x01, 
	STATUS_LOGGEDIN			= 0x02, 
	STATUS_TRANSFER_PENDING = 0x04, 
	STATUS_NEVER			= 0x08, 
	STATUS_PROTECTED		= 0x10  
};

class WorldPacket;

struct OpcodeHandler
{
	char const* name;
	unsigned long status;
	void (WorldSession::*handler)(WorldPacket& recvPacket);
};

extern OpcodeHandler opcodeTable[NUM_MSG_TYPES];


inline const char* LookupOpcodeName(uint16 id)
{
	if (id >= NUM_MSG_TYPES)
		return "Received unknown opcode, it's more than max!";
	return opcodeTable[id].name;
}
#endif