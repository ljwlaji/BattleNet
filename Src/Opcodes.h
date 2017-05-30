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

#ifndef _OPCODES_H
#define _OPCODES_H

#include "Common.h"

// Note: this include need for be sure have full definition of class WorldSession
//       if this class definition not complite then VS for x64 release use different size for
//       struct OpcodeHandler in this header and Opcode.cpp and get totally wrong data from
//       table opcodeTable in source when Opcode.h included but WorldSession.h not included
#include "WorldSession.h"

// List of Opcodes
enum Opcodes
{
	AUTH_NERVER = 0x000,
	CMSG_LOGIN = 0x001,
};

// Don't forget to change this value and add opcode name to Opcodes.cpp when you add new opcode!
#define NUM_MSG_TYPES 0x424

/// Player state
enum SessionStatus
{
	STATUS_AUTHED = 0x01, //!< Player authenticated
	STATUS_LOGGEDIN = 0x02, //!< Player in game
	STATUS_TRANSFER_PENDING = 0x04, //!< Player transferring to another map
	STATUS_NEVER = 0x08, //!< Opcode not accepted from client (deprecated or server side only)
	STATUS_PROTECTED = 0x10  //!< Using this opcode is time protected
};

class WorldPacket;

struct OpcodeHandler
{
	char const* name;
	unsigned long status;
	void (WorldSession::*handler)(WorldPacket& recvPacket);
};

extern OpcodeHandler opcodeTable[NUM_MSG_TYPES];

// Lookup opcode name for human understandable logging
inline const char* LookupOpcodeName(uint16 id)
{
	if (id >= NUM_MSG_TYPES)
		return "Received unknown opcode, it's more than max!";
	return opcodeTable[id].name;
}
#endif