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
	bool							IsNpc(uint64 _myId);
	bool							DoNpcRandomMove(GameSessionRef& _session);
	
public:
	vector<int>						GetRandomDirectionIndices();

	void							RemoveViewPlayer(uint64 _id) { WRITE_LOCK; viewPlayer.erase(_id); };

public:
	void							SetCurrentPlayer(const PlayerRef& _player) { currentPlayer = _player; }
	const PlayerRef&				GetCurrentPlayer() const { return currentPlayer; }

	void							SetViewPlayer(const unordered_set<uint64_t>& _players) { viewPlayer = _players; }
	const unordered_set<uint64_t>&	GetViewPlayer() const { return viewPlayer; }
	void							AddViewPlayer(const uint64 _id) { WRITE_LOCK; viewPlayer.insert(_id); }
	void							SetRoom(const weak_ptr<Room>& _roomPtr) { room = _roomPtr; }
	weak_ptr<Room>					GetRoom() const { return room; }
	uint64							GetId() const { return myId; }
	void							SetId(uint64 _id) { myId = _id; }


	void							RoomReset() { room.reset(); };

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
private:
	const array<pair<int, int>, 4> directions = { { {0,1}, {0,-1}, {1, 0}, {-1, 0} } };

};

