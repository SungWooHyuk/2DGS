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


struct STAT
{
	int32	level;
	int32	hp;
	int32	mp;
	int32	exp;
	int32	maxHp;
	int32	maxMp;
	int32	maxExp;
};

struct SAVEDB
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
};
struct POS
{
	int32	posx;
	int32	posy;

	bool operator==(const POS& other) const {
		return posx == other.posx && posy == other.posy;
	}
};

struct TP
{
	chrono::system_clock::time_point	attackTime;
	chrono::system_clock::time_point	attackEndTime;
	chrono::system_clock::time_point	moveTime;
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

#define	MAPDATA				MapData::GetInstance()
#define	SFSYSTEM			SFSystem::GetInstance()
#define TILE				Tile::GetInstance()
#define DB					DataBase::GetInstance()
#define ROOMMANAGER			GRoomManager
#define GAMESESSIONMANAGER	GGameSessionManager

#define PLAYER(id)			(sessions[(id)]->GetCurrentPlayer())
#define PLSTAT(id)   		(sessions[(id)]->GetCurrentPlayer())->GetStat()
#define PLAYERSTAT			player->GetStat()

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
#endif 