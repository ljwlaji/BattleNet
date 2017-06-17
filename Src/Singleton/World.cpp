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
	sLog->OutWarning("读取工会信息...");
	LoadGuildMemberTemplate();
	sLog->OutWarning("读取新闻信息...");
	LoadNewsTemplate();
	sLog->OutWarning("读取活动信息...");
	LoadActionTemplate();
	sLog->OutWarning("读取语言库信息...");
	LoadLocalItemStrings();
	sLog->OutWarning("读取物品图标信息...");
	LoadItemDisplayInfo();
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
	m_ItemTemplatMap.clear();
	Result _res;//											0	  1		 2		3		4		5		6			7				8			9				10		11
	if (sDataBase->GetResult(WorldDataBase, _res, "SELECT entry,class,subclass,name,Quality,BuyPrice,SellPrice,InventoryType,AllowableClass,ItemLevel,RequiredLevel,displayid FROM item_template"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			_SingleLine = itr->second;
			ItemTemplate _ItemTemplate;
			_ItemTemplate.Entry						= _SingleLine.at(0).GetInt();
			_ItemTemplate.Class						= _SingleLine.at(1).GetInt();
			_ItemTemplate.Subclass					= _SingleLine.at(2).GetInt();
			_ItemTemplate.Name						= _SingleLine.at(3).GetString();
			_ItemTemplate.Quality					= _SingleLine.at(4).GetInt();
			_ItemTemplate.BuyPrice					= _SingleLine.at(5).GetInt();
			_ItemTemplate.SellPrice					= _SingleLine.at(6).GetInt();
			_ItemTemplate.InventoryType				= _SingleLine.at(7).GetInt();
			_ItemTemplate.AllowableClass			= _SingleLine.at(8).GetInt();
			_ItemTemplate.ItemLevel					= _SingleLine.at(9).GetInt();
			_ItemTemplate.RequiredLevel				= _SingleLine.at(10).GetInt();
			_ItemTemplate.DisPlayid					= _SingleLine.at(11).GetInt();
			m_ItemTemplatMap[_ItemTemplate.Entry]	= _ItemTemplate;
		}
		sLog->OutSuccess("读取 %d 件物品", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadItemTemplate()");
}

uint32 World::GetAuctionItems(uint32 Teamid, uint32 PageId, std::map<uint32, AuctionItem>& TempMap, std::string& ItemName)
{
	// human 1 
	// Alliance Generic 2
	// n-elf 3

	// orc, and generic for horde 6
	// trolls 5
	// undead 4

	// booty bay, neutral 7
	// gadgetzan, neutral 7
	// everlook, neutral 7
	TempMap.clear();
	Result _res;//											   0	1		2			3			4			5		6	7		8		9		  10
	switch (Teamid)
	{
	case 469: //ALL
		sDataBase->GetResult(CharacterDataBase, _res, "SELECT id,houseid,itemguid,item_template,itemowner,buyoutprice,time,buyguid,lastbid,startbid FROM auction WHERE houseid in (1,2,3) ORDER BY id");
		break;
	case 67://Horde
		sDataBase->GetResult(CharacterDataBase, _res, "SELECT id,houseid,itemguid,item_template,itemowner,buyoutprice,time,buyguid,lastbid,startbid FROM auction WHERE houseid in (4,5,6) ORDER BY id");
		break;
	default:
		sDataBase->GetResult(CharacterDataBase, _res, "SELECT id,houseid,itemguid,item_template,itemowner,buyoutprice,time,buyguid,lastbid,startbid FROM auction WHERE houseid = 7 ORDER BY id");
		break;
	}
	std::vector<RowInfo> _SingleLine;
	uint32 ResultItem = 0;
	uint32 CurrentPos = 0;
	uint32 ItemNumber = 0;
	uint32 StartPos = (PageId - 1) * 5;
	for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
	{
		_SingleLine = itr->second;
		if (!ItemName.empty())
		{
			const Local_Item_String* Template = GetItemLocalString(_SingleLine.at(3).GetInt());
			//判断玩家地区信息
			if (!Template || Template->Name_Chinese_Normal != ItemName)
				continue;
		}
		ItemNumber++;
		if (CurrentPos++ < StartPos || ResultItem++ >= 5)
			continue;
		AuctionItem _AuctionItem;
		_AuctionItem.id				= _SingleLine.at(0).GetInt();
		_AuctionItem.item_template	= _SingleLine.at(3).GetInt();
		_AuctionItem.buyout_price	= _SingleLine.at(5).GetInt();
		_AuctionItem.last_bid		= _SingleLine.at(8).GetInt();
		TempMap[_AuctionItem.id]	= _AuctionItem;
		//}
		//else
		//{
		//	ItemNumber++;
		//	if (ResultItem >= 5)
		//		break;
		//	if (CurrentPos++ < (PageId - 1) * 5)
		//		continue;
		//
		//	AuctionItem _AuctionItem;
		//	_AuctionItem.id					= _SingleLine.at(0).GetInt();
		//	_AuctionItem.item_template		= _SingleLine.at(3).GetInt();
		//	_AuctionItem.buyout_price		= _SingleLine.at(5).GetInt();
		//	_AuctionItem.last_bid			= _SingleLine.at(8).GetInt();
		//	TempMap[_AuctionItem.id]		= _AuctionItem;
		//	ResultItem++;
		//}
	}
	return ItemName.empty() ? _res.size() : ItemNumber;
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

void World::LoadNewsTemplate()
{
	m_NewsTemplate.clear();
	Result _res;
	int k = 0;
	if (sDataBase->GetResult(WorldDataBase, _res, "SELECT time,title,text FROM playfun_battlenet_news"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			_SingleLine = itr->second;
			NewInfoTemplate _NewInfoTemplate;
			_NewInfoTemplate.Time						= _SingleLine.at(0).GetInt();
			_NewInfoTemplate.Title						= _SingleLine.at(1).GetString();
			_NewInfoTemplate.Message					= _SingleLine.at(2).GetString();
			m_NewsTemplate[k++]							= _NewInfoTemplate;
		}
		sLog->OutSuccess("读取 %d 件新闻信息", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadNewsTemplate()");
}

void World::LoadActionTemplate()
{
	m_ActionImageList.clear();
	Result _res;
	int k = 0;
	if (sDataBase->GetResult(WorldDataBase, _res, "SELECT url FROM playfun_battlenet_actions"))
	{
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			m_ActionImageList.push_back(itr->second.at(0).GetString());
		}
		sLog->OutSuccess("读取 %d 件活动信息", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadActionTemplate()");
}

WorldSession* World::CreateSessionForNewSocket(const uint32 & Socket, std::string& address, const uint8& SocketPage)
{
	WorldSession* NewSession = nullptr;
	ThreadLocker loc(SessionMapLock);
	if (m_SessionMap.find(Socket) != m_SessionMap.end())
		NewSession = m_SessionMap[Socket];
	else NewSession = new WorldSession(Socket, address, SocketPage);
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

const GuildTemplate* World::GetGuildInfoById(const uint32& guid_id)
{
	if (m_GuidMap.find(guid_id) != m_GuidMap.end())
		return &m_GuidMap[guid_id];
	return nullptr;
}

const NewInfoTemplate* World::GetNewsInfo(const uint32 & Number)
{
	if (m_NewsTemplate.find(Number) != m_NewsTemplate.end())
		return &m_NewsTemplate[Number];
	return nullptr;
}

const ItemTemplate * World::GetItemTemplate(const uint32 & ItemEntry)
{
	if (m_ItemTemplatMap.empty() || m_ItemTemplatMap.find(ItemEntry) == m_ItemTemplatMap.end())
		return nullptr;
	return &m_ItemTemplatMap[ItemEntry];
}

const std::list<std::string>* World::GetActionInfo()
{
	if (m_ActionImageList.empty())
		return nullptr;
	return &m_ActionImageList;
}

const Local_Item_String * World::GetItemLocalString(const uint32 & ItemEntry)
{
	if (m_LocalItemStrings.find(ItemEntry) != m_LocalItemStrings.end())
		return &m_LocalItemStrings[ItemEntry];
	return nullptr;
}

std::string World::GetItemUrl(const uint32 & ItemEntry)
{
	if (m_ItemDisplayInfo.find(ItemEntry) != m_ItemDisplayInfo.end())
		return m_ItemDisplayInfo[ItemEntry];
	return "";
}

SingleBattleNetAccount* World::GetBattleNetInfo(const uint32 & account_id)
{
	if (BattleNetAccounts.find(account_id) != BattleNetAccounts.end())
		return &BattleNetAccounts[account_id];
	return nullptr;
}

void World::update(const uint32& diff)
{
	ThreadLocker loc(SessionMapLock);
	for (SessionMap::iterator itr = m_SessionMap.begin(); itr != m_SessionMap.end(); itr++)
		if (WorldSession* single_session = itr->second)
		{
			if (single_session->IsClosing())
			{
				RemoveItr.push_back(itr);
				continue;
			}
			single_session->Update(diff);
		}

	while (RemoveItr.size())
	{
		if (WorldSession* single_session = (*RemoveItr.begin())->second)
		{
			m_SessionMap.erase(*RemoveItr.begin());
			RemoveItr.pop_front();
			delete single_session;
		}
	}
}

World::World()
{

}

World::~World()
{
	_World = nullptr;
}

void World::LoadItemDisplayInfo()
{
	m_ItemDisplayInfo.clear();
	Result _res;
				//											0		1
	if (sDataBase->GetResult(WorldDataBase, _res, "SELECT field0,field5 FROM item_display_info"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
			m_ItemDisplayInfo[itr->second.at(0).GetInt()] = itr->second.at(1).GetString();
		sLog->OutSuccess("读取 %d 条物品图标信息", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadItemDisplayInfo()");
}

void World::LoadLocalItemStrings()
{
	m_LocalItemStrings.clear();
	Result _res;
	int k = 0;//											0		1			2		3		4			5			6		7		8			9
	if (sDataBase->GetResult(WorldDataBase, _res, "SELECT entry,name_loc1,name_loc2,name_loc3,name_loc4,name_loc5,name_loc6,name_loc7,name_loc8,description_loc1 FROM locales_item"))
	{
		std::vector<RowInfo> _SingleLine;
		for (Result::iterator itr = _res.begin(); itr != _res.end(); itr++)
		{
			_SingleLine = itr->second;
			Local_Item_String _Local_Item_String;
			_Local_Item_String.Name_Korean					= _SingleLine.at(1).GetString();
			_Local_Item_String.Name_France					= _SingleLine.at(2).GetString();
			_Local_Item_String.Name_German					= _SingleLine.at(3).GetString();
			_Local_Item_String.Name_Chinese_Normal			= _SingleLine.at(4).GetString();
			_Local_Item_String.Name_Chinese_Traditional		= _SingleLine.at(5).GetString();
			_Local_Item_String.Name_Spanish					= _SingleLine.at(6).GetString();
			_Local_Item_String.Name_Russan					= _SingleLine.at(8).GetString();
			m_LocalItemStrings[_SingleLine.at(0).GetInt()]	= _Local_Item_String;
		}
		sLog->OutSuccess("读取 %d 条物品区域语言信息", _res.size());
	}
	else
		sLog->OutBug("Load Template Error On void World::LoadLocalItemStrings()");
}
