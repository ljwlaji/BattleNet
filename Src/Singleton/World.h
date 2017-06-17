#ifndef __WORLD_H__
#define __WORLD_H__
#include "Common.h"

#define sWorld World::GetInstance()

struct Local_Item_String
{
	std::string Name_Korean;
	std::string Name_France;
	std::string Name_German;
	std::string Name_Chinese_Normal;
	std::string Name_Chinese_Traditional;
	std::string Name_Spanish;
	std::string Name_Russan;
};

struct NewInfoTemplate
{
	std::string Title;
	uint32 Time;
	std::string Message;
};

struct GuildTemplate
{
	uint32 guildid;
	uint32 name;
	uint32 leaderguid;
	uint32 info;
	uint32 motd;
	uint32 createdate;
};

struct AuctionItem
{
	uint32 id;
	uint32 houseid;
	uint32 itemguid;
	uint32 item_template;
	uint32 item_owner;
	uint32 buyout_price;
	uint32 time;
	uint32 buy_guid;
	uint32 last_bid;
	uint32 start_bid;
	uint32 deposit;
};

struct ItemTemplate
{
	uint32 Entry;
	uint32 Class;
	uint32 Subclass;
	std::string Name;
	uint8  Quality;
	uint32 BuyPrice;
	uint32 SellPrice;
	uint32 InventoryType;
	uint32 AllowableClass;
	uint32 AllowableRace;
	uint32 ItemLevel;
	uint32 RequiredLevel;
	uint32 DisPlayid;
};
class WorldSession;

struct SingleBattleNetAccount
{
	std::string BattleNetUserName;
	std::string BattleNetPassWord;
	std::string Name;
	uint32 CharacterToShow;
	bool IsOnline;
	uint32 guild_id;
	WorldSession* m_session;

};

typedef std::map<uint32, WorldSession*> SessionMap;
class World
{
public:
	static World* GetInstance();
	void OnStart();
	WorldSession* CreateSessionForNewSocket(const uint32& Socket,std::string& address,const uint8& socketpage);
	WorldSession* GetSessionBySocket(const uint32& Socket);
	SingleBattleNetAccount* GetBattleNetInfo(const uint32& account_id);
	const GuildTemplate* GetGuildInfoById(const uint32& guid_id);
	const NewInfoTemplate* GetNewsInfo(const uint32& Number);
	const ItemTemplate* GetItemTemplate(const uint32& ItemEntry);
	const std::list<std::string>* GetActionInfo();
	const Local_Item_String* GetItemLocalString(const uint32& ItemEntry);
	std::string GetItemUrl(const uint32& ItemEntry);
	uint32 GetAuctionItems(uint32 Teamid, uint32 PageId, std::map<uint32, AuctionItem>& TempMap, std::string& ItemName);
private:
	World();
	~World();
	void LoadItemDisplayInfo();
	void LoadLocalItemStrings();
	void LoadBattleNetAccounts();
	void LoadItemTemplate();
	void LoadGuildMemberTemplate();
	void LoadNewsTemplate();
	void LoadActionTemplate();
public:
	void update(const uint32& diff);

protected:
	SessionMap m_SessionMap;
private:
	mutex SessionMapLock;
	std::map<uint32, SingleBattleNetAccount> BattleNetAccounts;
	std::map<uint32, ItemTemplate> m_ItemTemplatMap;
	std::map<uint32, AuctionItem> m_AcutionMap;
	std::map<uint32, GuildTemplate> m_GuidMap;
	std::map<uint32, NewInfoTemplate> m_NewsTemplate;
	std::map<uint32, Local_Item_String> m_LocalItemStrings;
	std::map<uint32, std::string> m_ItemDisplayInfo;
	std::list<SessionMap::iterator> RemoveItr;
	std::list<std::string> m_ActionImageList;
};

#endif