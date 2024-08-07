#pragma once

#include "utils.h"
#include "GameSession.h"
#include "Room.h"

class Player
{
public:
	Player();
	~Player();
	Player(string _name, STAT _stat, POS _pos, S_STATE _state, uint32 _room, Protocol::PlayerType _pt);
public:
	void					LevelUp();

public:
	void					SetId(uint64 _id) { myId = _id; };
	uint64					GetId() const { return myId; };
	void					SetName(string _name) { myName = _name; };
	const string&			GetName() const { return myName; };
	
	void					SetStat(STAT _stat) { myStat = _stat; };
	const STAT&				GetStat() const { return myStat; };

	void					SetStatLevel(int32 _level) { myStat.level = _level; }
	void					SetStatHp(int32 _hp) { myStat.hp = _hp; }
	void					SetStatMp(int32 _mp) { myStat.mp = _mp; }
	void					SetStatExp(int32 _exp) { myStat.exp = _exp; }
	void					SetStatMaxHp(int32 _maxHp) { myStat.maxHp = _maxHp; }
	void					SetStatMaxMp(int32 _maxMp) { myStat.maxMp = _maxMp; }
	void					SetStatMaxExp(int32 _maxExp) { myStat.maxExp = _maxExp; }

	void					UpdateStatExp(int32 _exp) { myStat.exp += _exp; }
	void					UpdateStatHp(int32 _hp) { myStat.hp += _hp; }
	void					UpdateStatMp(int32 _mp) { myStat.mp += _mp; }

	void					SetPos(POS _pos) { myPos = _pos; };
	const POS&				GetPos() const { return myPos; };
	void					SetState(S_STATE _state) { myState = _state; };
	const S_STATE&			GetState() const { return myState; };
	void					SetCurrentroom(uint32 _room) { currentRoom = _room; };
	uint32					GetCurrentroom() const { return currentRoom; };
	void					SetPT(Protocol::PlayerType _pt) { PT = _pt; };
	Protocol::PlayerType	GetPT() const { return PT; };

	void					SetOwnerSession(GameSessionRef& _session) { ownerSession = _session; };
	GameSessionRef&			GetOwnerSession() { return ownerSession; };

private:
	GameSessionRef ownerSession;

private:
	uint64					myId;
	string					myName;
	STAT					myStat;
	POS						myPos;
	S_STATE					myState;
	uint32					currentRoom;
	Protocol::PlayerType	PT;

public:
	atomic<bool>			active;
};

