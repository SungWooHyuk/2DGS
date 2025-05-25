#include "pch.h"
#include "utils.h"
#include "RoomManager.h"
#include "Player.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Room.h"

#include <atomic>

shared_ptr<RoomManager> GRoomManager = make_shared<RoomManager>();

RoomManager::RoomManager()
{
	for (int i = 0; i < ROOM; ++i)
		for (int j = 0; j < ROOM; ++j)
			rooms[i][j] = MakeShared<Room>();
		
}

void RoomManager::EnterRoom(GameSessionRef& _session)
{
	GameSessionRef session = _session;
	PlayerRef player = session->GetCurrentPlayer();

	int32 roomX = player->POSX / ROOM_SIZE;
	int32 roomY = player->POSY / ROOM_SIZE;
	if (roomX >= 0 && roomX < 10 && roomY >= 0 && roomY < 10) {
		rooms[roomX][roomY]->DoAsync(&Room::Enter, player);
		player->SetCurrentroom(GetRoomNumber(player->POSX, player->POSY));
		session->SetRoom(rooms[roomX][roomY]);
	}
	else {
		GLogger::Log(spdlog::level::err, "[RoomManager][EnterRoom] out bound");
	}
}

void RoomManager::LeaveRoom(GameSessionRef& _session)
{
	GameSessionRef session = _session;
	PlayerRef player = session->GetCurrentPlayer();

	int roomX = player->POSX / ROOM_SIZE;
	int roomY = player->POSY / ROOM_SIZE;

	if (player->GetCurrentroom() != -1) {
		rooms[roomX][roomY]->DoAsync(&Room::Leave, player);
		session->RoomReset();
		player->SetCurrentroom(9999);
	}
}


vector<RoomRef> RoomManager::GetAdjacentRooms(uint32 _roomNumber)
{
	vector<RoomRef> adjacentRooms;
	int row = _roomNumber / ROOM;
	int col = _roomNumber % ROOM;

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			if (i == 0 && j == 0)
				continue;

			int newRow = row + i;
			int newCol = col + j;

			if (newRow >= 0 && newRow < ROOM && newCol >= 0 && newCol < ROOM) {
				adjacentRooms.push_back(rooms[newRow][newCol]);
			}
		}
	}

	return adjacentRooms;
}


unordered_set<uint64> RoomManager::ViewList(GameSessionRef& _session, bool _npc)
{
	
	unordered_set<uint64> viewPlayer;
	PlayerRef player = _session->GetCurrentPlayer();
	vector<RoomRef> vroom;

	{ 
		READ_LOCK;
		vroom = GetAdjacentRooms(player->GetCurrentroom());
		vroom.push_back(_session->GetRoom().lock());
	}

	for (const auto& room : vroom)
	{
		auto players = room->GetPlayersCopy();

		for (const auto& other : players)
		{
			if (other.second)
			{
				PlayerRef vl = other.second;
				if (vl->GetState() == ST_INGAME)
				{
					if (vl->GetPT() == Protocol::PLAYER_TYPE_CLIENT) // p-p
					{
						if (CanSee(player, vl))
							viewPlayer.insert(vl->GetId());
					}
					else if (vl->GetPT() == Protocol::PLAYER_TYPE_DUMMY && player->GetPT() == Protocol::PLAYER_TYPE_CLIENT) // p- d
					{
						if (CanSee(player, vl))
							viewPlayer.insert(vl->GetId());
					}
					else {
						if (!_npc && player->GetPT() == Protocol::PLAYER_TYPE_CLIENT) // p - n
						{
							if (CanSee(player, vl)) {
								viewPlayer.insert(vl->GetId());
								GAMESESSIONMANAGER->WakeNpc(vl, player);
							}
						}
					}
				}
			}
		}
	}

	return viewPlayer;
}

uint32 RoomManager::GetRoomNumber(uint32 _x, uint32 _y)
{

	if (_x < 0 || _y >= 2000 || _y < 0 || _x >= 2000)
		return -1;

	const uint32 numRow = ROOM;
	const uint32 numCol = ROOM;

	int col = _x / (W_WIDTH / numRow);
	int row = _y / (W_HEIGHT / numCol);

	return row * numCol + col;
}

bool RoomManager::CanSee(PlayerRef _from, PlayerRef _to)
{
	if (_from->GetId() == _to->GetId())
		return false;

	int deltaX = std::abs(_from->POSX - _to->POSX);
	if (deltaX > VIEW_RANGE) return false;

	int deltaY = std::abs(_from->POSY - _to->POSY);
	return deltaY <= VIEW_RANGE;
}
