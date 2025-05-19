/*------------------------
	Related Windows Size
--------------------------*/
#ifndef UTILS_H
#define UTILS_H

#include "pch.h"

constexpr uint32 MAX_USER = 10000;
constexpr uint32 MAX_NPC = 20000;

constexpr uint32 NAME_LEN = 20;
constexpr uint32 VIEW_RANGE = 7;

constexpr uint32 SCREEN_WIDTH = 16;
constexpr uint32 SCREEN_HEIGHT = 16;

constexpr uint32 TILE_WIDTH = 65;

constexpr uint32 WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr uint32 WINDOW_HEIGHT = SCREEN_HEIGHT * TILE_WIDTH;

constexpr uint32 TOWN_SIZE = 20;
constexpr uint32 W_WIDTH = 2000;
constexpr uint32 W_HEIGHT = 2000;

constexpr int PORT_NUM = 4000;
constexpr int DB_PORT = 5000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 100;

constexpr uint32 ROOM_SIZE = W_WIDTH / 10;
constexpr uint32 ROOM = 10;

extern uint32	g_left_x;
extern uint32	g_top_y;

constexpr int MAX_TEST = 10000;
constexpr int MAX_CLIENTS = MAX_TEST * 2;
constexpr int INVALID_ID = -1;
constexpr int DELAY_LIMIT = 100;
constexpr int DELAY_LIMIT2 = 150;
constexpr int ACCEPT_DELY = 50;
constexpr int FORCE_SAVE_THRESHOLD = 20;

enum
{
	WORKER_TICK = 64
};

enum S_STATE
{
	ST_FREE,
	ST_ALLOC,
	ST_INGAME,
	ST_SLEEP
};

struct POS
{
	int32	posx;
	int32	posy;

	bool operator==(const POS& other) const {
		return posx == other.posx && posy == other.posy;
	}

	POS operator+(const POS& other) const {
		return { posx + other.posx, posy + other.posy };  
	}

	bool operator<(const POS& other) const {
		return std::tie(posy, posx) < std::tie(other.posy, other.posx); 
	}

	bool operator>(const POS& other) const {
		return std::tie(posy, posx) > std::tie(other.posy, other.posx); 
	}

};


struct PQNode
{
	int32 f; // f = g + h
	int32 g;
	POS pos;

	bool operator<(const PQNode& other) const { return f < other.f; }
	bool operator>(const PQNode& other) const { return f > other.f; }

};

enum { DIR_COUNT = 4 };

constexpr int32 cost[8] =
{
	10,
	10,
	10,
	10,
	14,
	14,
	14,
	14
};

constexpr POS dirs[8] =
{	POS { -1,  0},		// UP
	POS {  0, -1},		// LEFT
	POS {  1,  0},		// DOWN
	POS {  0,  1},		// RIGHT
	POS { -1, -1},		// UP_LEFT
	POS {  1, -1},		// DOWN_LEFT
	POS {  1,  1},		// DOWN_RIGHT
	POS { -1,  1}		// UP_RIGHT 
};

constexpr array<pair<int, int>, 4> directions = { { {0,1}, {0,-1}, {1, 0}, {-1, 0} } };

struct STAT
{
	int32	level;
	int32	hp;
	int32	mp;
	int32	exp;
	int32	maxHp;
	int32	maxMp;
	int32	maxExp;

	int32	attackPower;
	int32	defencePower;
	int32	magicPower;
	int32	strength;
};

struct USER_INFO
{
	string	name;
	int32	posy;
	int32	posx;
	int32	exp;
	int32	hp;
	int32	mp;
	int32	level;
	int32	maxHp;
	int32	maxMp;
	int32	maxExp;
	int32	gold;
};

struct ITEM_INFO
{
	int32 itemId;
	string name;
	Protocol::InventoryTab itemType;
	Protocol::EquipmentSlot equipType;
	int32 effectType;
	int32 effectValue;
	int32 requiredLevel;
	string description;
	string iconPath;

	struct EQUIPMENT_INFO
	{
		int32 attackPower;
		int32 defensePower;
		int32 magicPower;
		int32 strength;
	} equipmentInfo;
};

struct USER_EQUIPMENT
{
	string userId;
	int32 slotWeapon;
	int32 slotHelmet;
	int32 slotTop;
	int32 slotBottom;
};

struct USER_INVENTORY
{
	string userId;
	int32 itemId;
	int32 quantity;
	Protocol::InventoryTab tab_type;
	int32 slot_index;
};

struct EQUIPMENT
{
	int32 slotWeapon;
	int32 slotHelmet;
	int32 slotTop;
	int32 slotBottom;
};

struct INVEN
{
	int32						itemId;
	int32						quantity;
	Protocol::InventoryTab		tab_type;
	int32						slot_index;
};

enum class E_EQUIP
{
	NONE = 0,
	WEAPON = 1,
	HELMET = 2,
	TOP = 3,
	BOTTOM = 4
};
enum class E_INVEN
{
	NONE = 0,
	EQUIP = 1,
	CONSUM = 2,
	MISC = 3
};

enum class E_EFFECT_TYPE
{
	NONE = 0,
	HP = 1,
	MP = 2
};
struct TP
{
	chrono::system_clock::time_point	attackTime;
	chrono::system_clock::time_point	attackEndTime;
	chrono::system_clock::time_point	moveTime;
};

struct RankingData {
	string playerName;
	uint64 gold;

	bool operator==(const RankingData& other) const {
		return playerName == other.playerName && gold == other.gold;
	}
};

enum SystemText
{
	HP,
	MAXHP,
	MP,
	MAXMP,
	EXP,
	MAXEXP,
	LEVEL,
	NAME,
	PLAYERPOS,
	CHAT
};

enum SystemBox
{
	HPBOX,
	MPBOX,
	EXPBOX,
	HPINNERBOX,
	MPINNERBOX,
	EXPINNERBOX
};

struct InventoryKeyHash {
	size_t operator()(const pair<Protocol::InventoryTab, uint64>& key) const {
		return hash<int>()(static_cast<int>(key.first)) ^ (hash<uint64>()(key.second) << 1);
	}
};

struct InventoryKeyEqual {
	bool operator()(const pair<Protocol::InventoryTab, uint64>& lhs,
		const pair<Protocol::InventoryTab, uint64>& rhs) const {
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}
};

using InventoryKey = pair<Protocol::InventoryTab, uint64>;
using InventoryMap = unordered_map<InventoryKey, INVEN, InventoryKeyHash, InventoryKeyEqual>; // slot_index -> item
using EquipmentMap = unordered_map<Protocol::EquipmentSlot, ITEM_INFO>; // equip_slot -> item

struct DROPINFO
{
	uint64 itemId;
	uint64 minQuantity;
	uint64 maxQuantity;
	float  dropRate;
};

struct DROPTABLE
{
	vector<DROPINFO> drops;
};
#define	MAPDATA				MapData::GetInstance()
#define	SFSYSTEM			SFSystem::GetInstance()
#define TILE				Tile::GetInstance()
#define ROOMMANAGER			GRoomManager
#define GAMESESSIONMANAGER	GGameSessionManager
#define DBMANAGER			DBSessionManager::GetInstance()
#define ITEM				Item::GetInstance()
#define REDIS				RedisManager::GetInstance()
#define GDROPTABLE			DropTable::GetInstance()
#define DUMMYMANAGER		DummyManager::GetInstance()

#define PLAYERMOVETIME		session->GetPlayer()->GetTP().moveTime
#define NOW					chrono::system_clock::now()

#define POSX				GetPos().posx
#define POSY				GetPos().posy
#define	BOX					300, 30

#define WHITE				255, 255, 255
#define	RED					255, 0, 0
#define	BLUE				0, 0, 255
#define	MAGENTA				255, 0, 255
#define YELLOW				255, 255, 0
#define GREEN				0, 255, 0

#define CLIENT				0, 0, 65, 65
#define LV1MONSTER			0, 0, 65, 65
#define LV2MONSTER			455, 0, 65, 65
#define LV3MONSTER			0, 260, 65, 65
#define LV4MONSTER			260, 260, 65, 65

#define LV1STAT				1, 10, 0, 2, 10, 0, 0
#define LV2STAT				2, 20, 0, 4, 20, 0, 0
#define LV3STAT				3, 30, 0, 6, 30, 0, 0
#define LV4STAT				4, 40, 0, 8, 40, 0, 0

#define	OUTLINETHICK		3.f

#define INFORMATION_X		1050
#define	INFORMATION_HP_Y	400
#define	INFORMATION_MP_Y	500
#define	INFORMATION_EXP_Y	600

#define NO_MOVE				-1
#define UP					1
#define DOWN				2
#define RIGHT				3
#define LEFT				4

#define SCRACH				1
#define TOWN				1010,1010

#define GOLD				9999
#define MAXCONSUME			99
#define MAXMISC				200
#define IP					L"127.0.0.1"
#endif 