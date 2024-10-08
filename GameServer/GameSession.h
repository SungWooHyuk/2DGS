#pragma once
#include "Session.h"
#include "utils.h"
#include "Room.h"

class GameSession : public PacketSession
{
public:
	GameSession() {};
	~GameSession() { cout << "~GameSession" << endl; };

public:

	virtual void					OnConnected() override;
	virtual void					OnDisconnected() override;
	virtual void					OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void					OnSend(int32 len) override;

public:
	vector<int>						GetRandomDirectionIndices();

	void							RemoveViewPlayer(uint64 _id) { WRITE_LOCK; viewPlayer.erase(_id); };
	void							SetViewPlayer(const unordered_set<uint64_t>& _players) { WRITE_LOCK; viewPlayer = _players; }
	const unordered_set<uint64_t>& GetViewPlayer() { READ_LOCK; return viewPlayer; }
	void							AddViewPlayer(const uint64 _id) { WRITE_LOCK; viewPlayer.insert(_id); }
	void							ResetViewPlayer() { WRITE_LOCK; viewPlayer.clear(); }


public:
	void							SetCurrentPlayer(const PlayerRef& _player) { WRITE_LOCK; currentPlayer = _player; }
	const PlayerRef& GetCurrentPlayer() const { return currentPlayer; }

	void							SetRoom(const weak_ptr<Room>& _roomPtr) { room = _roomPtr; }
	weak_ptr<Room>					GetRoom() const { return room; }
	uint64							GetId() const { return myId; }
	void							SetId(uint64 _id) { myId = _id; }
	void							RoomReset() { room.reset(); };
public:
	bool							CanGo(POS _pos);
	void							ResetPath();
	void							SetPath(POS _dest, map<POS, POS>& _parent);
	bool							EmptyPath();
	uint32							GetPathIndex() { READ_LOCK; return pathIndex; };
	void							SetPathIndex(uint32 _path) { WRITE_LOCK; pathIndex = _path; };
	vector<POS>						GetPath() { READ_LOCK; return path; };
	uint32							GetPathCount() { READ_LOCK; return pathCount; };
	void							SetPathCount(uint32 _count) { WRITE_LOCK; pathCount = _count; };

public:
	void							RemovePkt(uint64 _id);
	void							RespawnPkt(int32 _hp, POS _pos, int32 _exp);
	void							AddObjectPkt(Protocol::PlayerType _pt, uint64 _id, string _name, POS _pos);
	void							MovePkt(uint64 _id, POS _pos, int64 _time);
	void							DamagePkt(int32 _damage);
	void							ChatPkt(uint64 _id, string _mess);
	void							StatChangePkt(int32 _level, int32 _hp, int32 _maxhp, int32 _mp, int32 _maxmp, int32 _exp, int32 _maxexp);
	void							LoginPkt(bool _success, uint64 _id, Protocol::PlayerType _pt, string _name, POS _pos, STAT _stat);
	void							LoginPkt(bool _success);

private:
	USE_LOCK;
	PlayerRef currentPlayer;
	unordered_set<uint64> viewPlayer;
	weak_ptr<Room> room;

	uint64			myId;

	vector<POS>		path;
	uint32			pathIndex = 1;
	uint32			pathCount = 0;

private:
	const array<pair<int, int>, 4> directions = { { {0,1}, {0,-1}, {1, 0}, {-1, 0} } };

};

