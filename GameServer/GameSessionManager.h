#pragma once

#include "JobQueue.h"
#include "utils.h"

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;

class GameSessionManager : public JobQueue
{
public:
	GameSessionManager() {};
	~GameSessionManager() {};

	void			Add(GameSessionRef _session);
	void			Remove(GameSessionRef _session);
	void			Broadcast(SendBufferRef _sendBuffer);
	void			Respawn(GameSessionRef _session);
	void			PlayerRespawn(uint64 _id);

	int				GetNewClientId();
	
	void			SaveAllPlayerStateSnap();
	void			SessionRankingUpdate();
	void			NpcRandomMove(uint64 _id);
	void			NpcAstarMove(uint64 _id);
	void			NpcAstarMoveTo(uint64 _id, uint64 _targetid);
	bool			DoNpcRandomMove(uint64 _id);
	bool			DoNpcAstarMove(uint64 _id);
	void			AstarMove(GameSessionRef& _session,PlayerRef& _player);
	void			NpcAstar(uint64 _id, uint64 _destId);
	void			Attack(GameSessionRef& _session, uint64 _id, uint64 _skill);
	void			Move(GameSessionRef& _session, uint64 _direction, int64 _movetime);
	void			Chat(GameSessionRef& _session, string _mess);
	bool			IsPlayer(uint64 _id);
	void			InitializeNPC();
	void			WakeNpc(PlayerRef _player, PlayerRef _toPlayer);
	GameSessionRef	GetSession(uint64 _id);
	//bool			UserInfoPlayer(uint64 _id);

public:
	void			SetupPlayerAndSession(int id, const string& name, const STAT& st, const POS& pos, Protocol::PlayerType pt);
	void			GenerateNPCAttributes(int id, string& name, STAT& st, POS& pos, Protocol::PlayerType& pt);


	void			UpdatePlayerPosition(PlayerRef& _player, uint64 _direction);
	void			HandleCollisions(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _new_vl);
	void			HandlePlayerDeath(GameSessionRef& _gamesession, PlayerRef& _player, uint64 _killerId);
	void			UpdateViewList(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl, int64 _movetime);

	void			HandleNpcCollision(PlayerRef& _player, uint64 _pl);
	void			HandleNPCViewListChanges(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl);
	void			MoveNpcToRandomPosition(GameSessionRef& _gamesession, PlayerRef& _player);

	bool			IsAdjacent(uint64 _attackerId, uint64 _targetId);
	void			HandleAttack(uint64 _attackerId, uint64 _targetId);
	void			HandleNPCDeath(uint64 _attackerId, uint64 _targetId);
	void			BroadcastAttackMessage(uint64 _attackerId, uint64 _targetId);
	
	void			DropItems(GameSessionRef& _gamesession,GameSessionRef& _usersession);
	void			UpdateGold(const string& _name, const int _newGold);
	void			UpdateGold();
public:
	USE_LOCK;
	Array<GameSessionRef, MAX_USER + MAX_NPC> sessions;


private:
	queue<uint64> freeId;
	
};

extern shared_ptr<GameSessionManager> GGameSessionManager;