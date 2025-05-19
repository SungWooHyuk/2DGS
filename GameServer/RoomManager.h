#pragma once

#include "pch.h"
#include "JobQueue.h"
#include "utils.h"

class Room;
using RoomRef = shared_ptr<Room>;

class RoomManager : public JobQueue
{
public:
	RoomManager();
	~RoomManager() {};

public:
	void							EnterRoom(GameSessionRef& _session);
	void							LeaveRoom(GameSessionRef& _session);

	uint32							GetRoomNumber(uint32 _x, uint32 _y);

	bool							CanSee(PlayerRef _from, PlayerRef _to);

	vector<RoomRef>					GetAdjacentRooms(uint32 _roomNumber);

	unordered_set<uint64>			ViewList(GameSessionRef& _session, bool _npc);

private:
	USE_LOCK;
	Array<Array<RoomRef, ROOM>, ROOM> rooms;
};

extern shared_ptr<RoomManager> GRoomManager;