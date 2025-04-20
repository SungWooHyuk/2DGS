#pragma once

#include "pch.h"
#include "JobQueue.h"
#include "utils.h"

class Room;

class RoomManager : public JobQueue
{
public:
	static RoomManager& GetInstance()
	{
		static RoomManager instance;
		return instance;
	}
public:
	void							EnterRoom(GameSessionRef& _session);
	void							LeaveRoom(GameSessionRef& _session);

	uint32							GetRoomNumber(uint32 _x, uint32 _y);

	bool							CanSee(PlayerRef _from, PlayerRef _to);

	vector<RoomRef>					GetAdjacentRooms(uint32 _roomNumber);

	unordered_set<uint64>			ViewList(GameSessionRef& _session, bool _npc);

private:
	RoomManager();
	~RoomManager() {};
	RoomManager(const RoomManager&) = delete;
	RoomManager& operator=(const RoomManager&) = delete;
private:
	USE_LOCK;
	Array<Array<RoomRef, ROOM>, ROOM> rooms;
};

