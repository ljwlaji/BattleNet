#include "DataBase.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include <iostream>
static World* _World = nullptr;

World* World::GetInstance()
{
	if (!_World)
		_World = new World();
	return _World;
}

void World::OnStart()
{
	sLog->OutWarning("***************************************************************************");
	sLog->OutLog("正在读取世界库信息....");
	sLog->OutWarning("读取战网角色信息...");
	LoadBattleNetAccounts();
	sLog->OutWarning("读取物品信息...");
	LoadItemTemplate();
	sLog->OutWarning("读取拍卖行信息...");
	LoadAuctionTemplate();
	sLog->OutWarning("读取工会信息...");
	LoadGuildMemberTemplate();
	sLog->OutSuccess("世界信息读取完毕...");
	sLog->OutWarning("***************************************************************************");
}

void World::LoadBattleNetAccounts()
{
	BattleNetAccounts.clear();
	Result _res;//													0				1				2		3			4
	if (sDataBase->GetResult(LoginDataBase, _res, "SELECT lingked_account_id,battle_net_username,password,name,linked_player_guid,is_online,guild_id FROM playfun_battle_net_accounts"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			_SingleLine = itr->second;
			SingleBattleNetAccount _SingleBattleNetAccount;
			_SingleBattleNetAccount.BattleNetUserName		= _SingleLine.at(1).GetString();
			_SingleBattleNetAccount.BattleNetPassWord		= _SingleLine.at(2).GetString();
			_SingleBattleNetAccount.Name					= _SingleLine.at(3).GetString();
			_SingleBattleNetAccount.CharacterToShow			= _SingleLine.at(4).GetInt();
			_SingleBattleNetAccount.IsOnline				= _SingleLine.at(5).GetBool();
			_SingleBattleNetAccount.guild_id				= _SingleLine.at(6).GetInt();
			BattleNetAccounts[_SingleLine.at(0).GetInt()]	= _SingleBattleNetAccount;

		}
		sLog->OutSuccess("读取 %d 条账号信息", _res.size());
	}
	else sLog->OutBug("Load Template Error On void World::LoadBattleNetAccounts()");
}

void World::LoadItemTemplate()
{
	BattleNetAccounts.clear();
	Result _res;//											0	  1		 2		3		4		5		6			7				8			9				10			11
	if (sDataBase->GetResult(WorldDataBase, _res, "SELECT entry,class,subclass,name,Quality,BuyPrice,SellPrice,InventoryType,AllowableClass,AllowableRace,ItemLevel,RequiredLevel FROM item_template"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			_SingleLine = itr->second;
			ItemTemplate _ItemTemplate;
			_ItemTemplate.Entry						= _SingleLine.at(0).GetInt();
			_ItemTemplate.Class						= _SingleLine.at(1).GetInt();
			_ItemTemplate.Subclass					= _SingleLine.at(2).GetInt();
			_ItemTemplate.Name						= _SingleLine.at(3).GetInt();
			_ItemTemplate.Quality					= _SingleLine.at(4).GetInt();
			_ItemTemplate.BuyPrice					= _SingleLine.at(5).GetInt();
			_ItemTemplate.SellPrice					= _SingleLine.at(6).GetInt();
			_ItemTemplate.InventoryType				= _SingleLine.at(7).GetInt();
			_ItemTemplate.AllowableClass			= _SingleLine.at(8).GetInt();
			_ItemTemplate.ItemLevel					= _SingleLine.at(9).GetInt();
			_ItemTemplate.RequiredLevel				= _SingleLine.at(10).GetInt();
			m_ItemTemplatMap[_ItemTemplate.Entry]	= _ItemTemplate;
		}
		sLog->OutSuccess("读取 %d 件物品", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadItemTemplate()");
}

void World::LoadAuctionTemplate()
{
	m_AcutionMap.clear();

	Result _res;//											   0	1		2			3			4			5		6	7		8		9		  10
	if (sDataBase->GetResult(CharacterDataBase, _res, "SELECT id,houseid,itemguid,item_template,itemowner,buyoutprice,time,buyguid,lastbid,startbid,deposit FROM auction"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			_SingleLine = itr->second;
			AuctionItem _AuctionItem;
			_AuctionItem.id					= _SingleLine.at(0).GetInt();
			_AuctionItem.houseid			= _SingleLine.at(1).GetInt();
			_AuctionItem.itemguid			= _SingleLine.at(2).GetInt();
			_AuctionItem.item_template		= _SingleLine.at(3).GetInt();
			_AuctionItem.item_owner			= _SingleLine.at(4).GetInt();
			_AuctionItem.buyout_price		= _SingleLine.at(5).GetInt();
			_AuctionItem.time				= _SingleLine.at(6).GetInt();
			_AuctionItem.buy_guid			= _SingleLine.at(7).GetInt();
			_AuctionItem.last_bid			= _SingleLine.at(8).GetInt();
			_AuctionItem.start_bid			= _SingleLine.at(9).GetInt();
			_AuctionItem.deposit			= _SingleLine.at(10).GetInt();
			m_AcutionMap[_AuctionItem.id]	= _AuctionItem;
		}
		sLog->OutSuccess("读取 %d 件拍卖信息", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadAuctionTemplate()");
}

void World::LoadGuildMemberTemplate()
{
	m_GuidMap.clear();
	Result _res;//											     0	   1		2		3	4		5		
	if (sDataBase->GetResult(CharacterDataBase, _res, "SELECT guildid,name,leaderguid,info,motd,createdate FROM guild"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			_SingleLine = itr->second;
			GuildTemplate _GuildTemplate;
			_GuildTemplate.guildid			= _SingleLine.at(0).GetInt();
			_GuildTemplate.name				= _SingleLine.at(1).GetInt();
			_GuildTemplate.leaderguid		= _SingleLine.at(2).GetInt();
			_GuildTemplate.info				= _SingleLine.at(3).GetInt();
			_GuildTemplate.motd				= _SingleLine.at(4).GetInt();
			_GuildTemplate.createdate		= _SingleLine.at(5).GetInt();
			m_GuidMap[_GuildTemplate.guildid] = _GuildTemplate;
		}
		sLog->OutSuccess("读取 %d 件工会信息", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadGuildMemberTemplate()");
}

WorldSession* World::CreateSessionForNewSocket(const uint32 & Socket, const char* buff)
{
	uint16 Lenth = *((uint8*)buff) - 8;
	uint16 Opcode = *((uint8*)(buff + 4));
	if (Opcode > 0x424 || Lenth > 0x1000)
		return nullptr;
	WorldPacket pack(Lenth, Opcode);
	pack.resize(Lenth);
	pack.put(0, (const unsigned char*)buff + 8, Lenth);
	WorldSession* NewSession = nullptr;
	
	ThreadLocker loc(SessionMapLock);
	if (m_SessionMap.find(Socket) != m_SessionMap.end())
		NewSession = m_SessionMap[Socket];
	else NewSession = new WorldSession(Socket);

	m_SessionMap[Socket] = NewSession;
	return NewSession;
}

WorldSession* World::GetSessionBySocket(const uint32 & Socket)
{
	ThreadLocker loc(SessionMapLock);
	if (m_SessionMap.find(Socket) != m_SessionMap.end())
		return m_SessionMap[Socket];
	return nullptr;
}

void World::update(const uint32& diff)
{
	ThreadLocker loc(SessionMapLock);
	for (SessionMap::iterator itr = m_SessionMap.begin(); itr != m_SessionMap.end(); itr++)
		if (WorldSession* single_session = itr->second)
			single_session->Update(diff);
}

World::World()
{

}

World::~World()
{
	_World = nullptr;
}