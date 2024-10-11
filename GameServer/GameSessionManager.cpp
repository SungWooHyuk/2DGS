#include "pch.h"
#include "utils.h"
#include <sstream>
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Player.h"
#include "MapData.h"
#include "RoomManager.h"
#include "DataBase.h"
#include "Protocol.pb.h"
#include <fstream>

shared_ptr<GameSessionManager> GGameSessionManager = make_shared<GameSessionManager>();

void GameSessionManager::Add(GameSessionRef _session)
{

	int newId = GetNewClientId();
	{
		WRITE_LOCK;
		_session->SetId(newId);
		sessions[newId] = _session;
	}
}

void GameSessionManager::Remove(GameSessionRef _session)
{
	GameSessionRef gamesession;
	{
		gamesession = _session;
		int id = gamesession->GetCurrentPlayer()->GetId();

		if (gamesession->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
			ASSERT_CRASH(SaveDBPlayer(id));

		unordered_set<uint64> vl = sessions[id]->GetViewPlayer();
		for (const auto& _vl : vl)
		{
			if (false == IsPlayer(_vl)) continue;
			if (_vl == id) continue;
			if (sessions[_vl]->GetCurrentPlayer()->GetState() != ST_INGAME) continue;
			if (sessions[_vl]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_DUMMY) continue;

			sessions[_vl]->RemovePkt(id);
			sessions[_vl]->RemoveViewPlayer(id);
		}
	}
	{

		freeId.push(gamesession->GetCurrentPlayer()->GetId());

		sessions[gamesession->GetCurrentPlayer()->GetId()]->GetCurrentPlayer()->SetState(ST_FREE);
		sessions[gamesession->GetCurrentPlayer()->GetId()].reset();
	}
}

void GameSessionManager::Broadcast(SendBufferRef _sendBuffer)
{
	READ_LOCK;
	for (GameSessionRef session : sessions)
	{
		if (session->GetCurrentPlayer()->GetId() < MAX_USER)
			session->Send(_sendBuffer);
	}
}

void GameSessionManager::Respawn(GameSessionRef _session)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _session;
		player = gamesession->GetCurrentPlayer();
	}

	ROOMMANAGER->EnterRoom(_session); // 룸 다시 입장.

	gamesession->s_lock.lock();
	player->SetStatHp(player->GetStat().maxHp);
	gamesession->s_lock.unlock();

	unordered_set<uint64> vl = ROOMMANAGER->ViewList(gamesession, true);
	gamesession->SetViewPlayer(vl);

	for (auto _vl : vl)
	{
		POS plPos = { gamesession->GetCurrentPlayer()->POSX, gamesession->GetCurrentPlayer()->POSY };

		sessions[_vl]->AddObjectPkt(
			gamesession->GetCurrentPlayer()->GetPT(),
			gamesession->GetCurrentPlayer()->GetId(),
			gamesession->GetCurrentPlayer()->GetName(),
			plPos
		);
	}

	{
		gamesession->s_lock.lock();
		gamesession->GetCurrentPlayer()->SetState(ST_INGAME);
		gamesession->s_lock.unlock();
	}
}

void GameSessionManager::PlayerRespawn(uint64 _id)
{
	POS pos{ TOWN };
	{
		sessions[_id]->s_lock.lock();
		sessions[_id]->GetCurrentPlayer()->SetStatHp(sessions[_id]->GetCurrentPlayer()->GetStat().maxHp);
		sessions[_id]->GetCurrentPlayer()->SetPos(pos);
		sessions[_id]->GetCurrentPlayer()->SetStatExp(sessions[_id]->GetCurrentPlayer()->GetStat().exp / 2);
		sessions[_id]->GetCurrentPlayer()->SetState(ST_INGAME);
		sessions[_id]->s_lock.unlock();
	}
	ROOMMANAGER->EnterRoom(sessions[_id]);
	sessions[_id]->RespawnPkt(sessions[_id]->GetCurrentPlayer()->GetStat().hp, pos, sessions[_id]->GetCurrentPlayer()->GetStat().exp);
}

int GameSessionManager::GetNewClientId()
{
	{
		WRITE_LOCK;
		if (!freeId.empty())
		{
			int newId = freeId.front();
			freeId.pop();
			return newId;
		}
	}

	for (int i = 0; i < MAX_USER; ++i)
	{
		READ_LOCK;
		if (sessions[i] == nullptr)
			return i;
	}

	return -1;
}

void GameSessionManager::WakeNpc(PlayerRef _player, PlayerRef _toPlayer)
{
	PlayerRef player;
	PlayerRef target;
	{
		player = _player;
		target = _toPlayer;
	}

	unordered_set<uint64> vl = ROOMMANAGER->ViewList(player->GetOwnerSession(), true);
	player->GetOwnerSession()->SetViewPlayer(vl);

	if (player->GetPT() == Protocol::PLAYER_TYPE_CLIENT) return;
	if (player->active) return; // 이미 깨어있으면 return
	bool old_state = false;
	if (false == atomic_compare_exchange_strong(&player->active, &old_state, true))
		return; // 한쓰레드만 접근하게 
	DoTimer(1000, &GameSessionManager::NpcAstarMove, player->GetId());
}

void GameSessionManager::NpcRandomMove(uint64 _id)
{
	GameSessionRef gamesession;
	PlayerRef npc;
	{
		gamesession = sessions[_id];
		npc = gamesession->GetCurrentPlayer();
	}

	if (gamesession->GetViewPlayer().size() > 0) {
		if (npc->GetState() == ST_INGAME)
		{
			DoNpcRandomMove(_id);
			DoTimer(1000, &GameSessionManager::NpcRandomMove, _id);
		}
		else
			npc->active = false;
	}
	else
		npc->active = false;
}

void GameSessionManager::NpcAstarMove(uint64 _id)
{
	PlayerRef npc;
	GameSessionRef gamesession;
	{
		gamesession = sessions[_id];
		npc = gamesession->GetCurrentPlayer();
	}

	unordered_set<uint64> viewPlayer = gamesession->GetViewPlayer();
	uint32 closestDistance = INT32_MAX;
	uint32 closestPlayerId = 0;

	if (viewPlayer.size() > 0) {
		if (gamesession->GetCurrentPlayer()->GetState() == ST_INGAME)
		{
			// 비어있거나(처음), 3회 이동후 가까운 플레이어 재 설정
			if (gamesession->EmptyPath() || gamesession->GetPathCount() > 3) 
			{
				gamesession->ResetPath();

				{
					for (auto id : viewPlayer)
					{
						if (sessions[id])
						{
							if (sessions[id]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT && sessions[id]->GetCurrentPlayer()->GetState() == ST_INGAME)
							{
								POS playerPos = sessions[id]->GetCurrentPlayer()->GetPos();
								uint32 distance = abs(npc->GetPos().posx - playerPos.posx) + abs(npc->GetPos().posy - playerPos.posy);
								if (distance < closestDistance) {
									closestDistance = distance;
									closestPlayerId = id;
								}
							}
						}
					}
				}

				NpcAstar(npc->GetId(), closestPlayerId);
			} 

			if(DoNpcAstarMove(_id))
				DoTimer(1000, &GameSessionManager::NpcAstarMove, _id);
		}
		else {
			npc->active = false;
			npc->GetOwnerSession()->ResetPath();
		}
	}
	else {
		npc->active = false;
		npc->GetOwnerSession()->ResetPath();
	}
}

void GameSessionManager::NpcAstarMoveTo(uint64 _id, uint64 _targetid)
{
	PlayerRef npc;
	GameSessionRef gamesession;
	{
		gamesession = sessions[_id];
		npc = gamesession->GetCurrentPlayer();
	}
	
	unordered_set<uint64> viewPlayer = gamesession->GetViewPlayer();
	uint32 closestDistance = INT32_MAX;
	uint32 closestPlayerId = 0;

	if (viewPlayer.size() > 0) {
		if (gamesession->GetCurrentPlayer()->GetState() == ST_INGAME)
		{
		
			NpcAstar(npc->GetId(), _targetid);

			if (DoNpcAstarMove(_id))
				DoTimer(1000, &GameSessionManager::NpcAstarMoveTo, _id, _targetid);
		}
		else {
			npc->active = false;
			npc->GetOwnerSession()->ResetPath();
		}
	}
	else {
		npc->active = false;
		npc->GetOwnerSession()->ResetPath();
	}
}

bool GameSessionManager::DoNpcRandomMove(uint64 _id)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = sessions[_id];
		player = gamesession->GetCurrentPlayer();
	}

	unordered_set<uint64> old_vl = gamesession->GetViewPlayer();

	MoveNpcToRandomPosition(gamesession, player);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession, true);

	HandleNPCViewListChanges(gamesession, player, old_vl, new_vl);

	return true;
}

bool GameSessionManager::DoNpcAstarMove(uint64 _id)
{

	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = sessions[_id];
		player = gamesession->GetCurrentPlayer();
	}

	unordered_set<uint64> old_vl = gamesession->GetViewPlayer();

	AstarMove(gamesession, player);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession, true);

	HandleNPCViewListChanges(gamesession, player, old_vl, new_vl);

	return true;
}

void GameSessionManager::AstarMove(GameSessionRef& _session, PlayerRef& _player)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _session;
		player = _player;
	}

	int index = gamesession->GetPathIndex();
	vector<POS> v = gamesession->GetPath();
	int size = v.size();

	gamesession->SetPathCount(gamesession->GetPathCount() + 1);

	if (!v.empty())
	{
		if (index >= size)
		{
			{
				player->GetOwnerSession()->s_lock.lock();
				player->SetPos(v[index - 1]);
				player->GetOwnerSession()->s_lock.unlock();
			}
		}
		else
		{
			{
				player->GetOwnerSession()->s_lock.lock();
				player->SetPos(v[index]);
				player->GetOwnerSession()->s_lock.unlock();
			}

			gamesession->SetPathIndex(index + 1);
		}
	}
}

void GameSessionManager::NpcAstar(uint64 _id, uint64 _destId)
{
	GameSessionRef gamesession;
	GameSessionRef dest_gamesession;
	PlayerRef player;
	{
		gamesession = sessions[_id];
		dest_gamesession = sessions[_destId];
		player = gamesession->GetCurrentPlayer();
	}

	if (gamesession->EmptyPath())
	{
		POS start = player->GetPos();
		POS dest;
		if (dest_gamesession)
			dest = dest_gamesession->GetCurrentPlayer()->GetPos();
		else
			return;
		
		vector<vector<bool>> closed(W_HEIGHT, vector<bool>(W_WIDTH, false));
		vector<vector<int>> best(W_HEIGHT, vector<int>(W_WIDTH, INT32_MAX));
		map<POS, POS> parent;
		priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;

		{
			int32 g = 0;
			int32 h = 10 * (abs(dest.posy - start.posy) + abs(dest.posx - start.posx));
			pq.push(PQNode{ g + h, g, start });
			best[start.posy][start.posx] = g + h;
			parent[start] = start;
		}

		// 가장 빠른길 찾기
		while (pq.empty() == false)
		{

			PQNode node = pq.top();
			pq.pop();

			if (closed[node.pos.posy][node.pos.posx])
				continue;
			if (best[node.pos.posy][node.pos.posx] < node.f)
				continue;

			closed[node.pos.posy][node.pos.posx] = true;

			if (node.pos == dest) // 도착
				break;
			
			for (int32 dir = 0; dir < DIR_COUNT; dir++)
			{
				POS nextPos = node.pos + dirs[dir];

				if (gamesession->CanGo(nextPos) == false)
					continue;
				if (closed[nextPos.posy][nextPos.posx])
					continue;

				int32 g = node.g + cost[dir];
				int32 h = 10 * (abs(dest.posy - nextPos.posy) + abs(dest.posx - nextPos.posx));

				if (best[nextPos.posy][nextPos.posx] <= g + h)
					continue;

				best[nextPos.posy][nextPos.posx] = g + h;
				pq.push(PQNode{ g + h, g, nextPos });
				parent[nextPos] = node.pos;

			}
		} 

		gamesession->SetPath(dest, parent);
		
	}
}

void GameSessionManager::Attack(GameSessionRef& _session, uint64 _id, uint64 _skill)
{
	GameSessionRef gamesession;
	{
		READ_LOCK;
		gamesession = _session;
	}
	unordered_set<uint64> vl = gamesession->GetViewPlayer();

	for (const auto& _vl : vl)
	{
		READ_LOCK;
		if (IsPlayer(_vl)) // Player ignore
			continue;

		if (IsAdjacent(_id, _vl))
			HandleAttack(_id, _vl);
	}

}

void GameSessionManager::Move(GameSessionRef& _session, uint64 _direction, int64 _movetime)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _session;
		player = gamesession->GetCurrentPlayer();
	}

	unordered_set<uint64> old_vl = gamesession->GetViewPlayer();;

	UpdatePlayerPosition(player, _direction);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession, false); // 움직인 후 주변애들 + WakeNPC까지 진행완료

	if (player->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
		HandleCollisions(gamesession, player, new_vl);

	gamesession->MovePkt(player->GetId(), { player->POSX, player->POSY }, _movetime);
	UpdateViewList(gamesession, player, old_vl, new_vl, _movetime);
}

void GameSessionManager::Chat(GameSessionRef& _session, string _mess)
{

}

bool GameSessionManager::IsPlayer(uint64 _id)
{
	return _id < MAX_USER;
}

bool GameSessionManager::SaveDBPlayer(uint64 _id)
{
	GameSessionRef gamesession;
	SAVEDB db;
	{
		gamesession = sessions[_id];
	}
	{
		db.level = gamesession->GetCurrentPlayer()->GetStat().level;
		db.hp = gamesession->GetCurrentPlayer()->GetStat().hp;
		db.maxHp = gamesession->GetCurrentPlayer()->GetStat().maxHp;
		db.mp = gamesession->GetCurrentPlayer()->GetStat().mp;
		db.maxMp = gamesession->GetCurrentPlayer()->GetStat().maxMp;
		db.exp = gamesession->GetCurrentPlayer()->GetStat().exp;
		db.maxExp = gamesession->GetCurrentPlayer()->GetStat().maxExp;
		db.name = gamesession->GetCurrentPlayer()->GetName();
		db.posx = gamesession->GetCurrentPlayer()->GetPos().posx;
		db.posy = gamesession->GetCurrentPlayer()->GetPos().posy;
	}
	return DB->SaveDB(db);
}

void GameSessionManager::UpdatePlayerPosition(PlayerRef& _player, uint64 _direction)
{
	PlayerRef player;
	{
		player = _player;
	}
	POS pos{ player->POSX, player->POSY };

	switch (_direction)
	{
	case LEFT:
		if (pos.posx > 0 && MAPDATA->GetTile(pos.posy, pos.posx - 1) != MAPDATA->e_OBSTACLE)
			--pos.posx;
		break;
	case RIGHT:
		if (pos.posx < W_WIDTH - 1 && MAPDATA->GetTile(pos.posy, pos.posx + 1) != MAPDATA->e_OBSTACLE)
			++pos.posx;
		break;
	case UP:
		if (pos.posy > 0 && MAPDATA->GetTile(pos.posy - 1, pos.posx) != MAPDATA->e_OBSTACLE)
			--pos.posy;
		break;
	case DOWN:
		if (pos.posy < W_HEIGHT - 1 && MAPDATA->GetTile(pos.posy + 1, pos.posx) != MAPDATA->e_OBSTACLE)
			++pos.posy;
		break;
	}

	{
		player->GetOwnerSession()->s_lock.lock();
		player->SetPos(pos);
		player->GetOwnerSession()->s_lock.unlock();
	}
}

void GameSessionManager::HandleCollisions(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _new_vl)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _gamesession;
		player = _player;
	}

	for (const auto& vl : _new_vl)
	{
		if (sessions[vl]->GetCurrentPlayer()->GetPT() != Protocol::PLAYER_TYPE_CLIENT && sessions[vl]->GetCurrentPlayer()->GetPT() != Protocol::PLAYER_TYPE_DUMMY)
		{
			if (sessions[vl]->GetCurrentPlayer()->GetPos() == player->GetPos())
			{
				{
					player->GetOwnerSession()->s_lock.lock();
					player->UpdateStatHp(sessions[vl]->GetCurrentPlayer()->GetStat().level * -5);
					player->GetOwnerSession()->s_lock.unlock();
				}

				if (player->GetStat().hp <= 0)
				{
					HandlePlayerDeath(gamesession, player, vl);
					return;
				}
				else
				{
					gamesession->StatChangePkt(
						player->GetStat().level,
						player->GetStat().hp,
						player->GetStat().maxHp,
						player->GetStat().mp,
						player->GetStat().maxMp,
						player->GetStat().exp,
						player->GetStat().maxExp
					);

					stringstream ss;
					ss << "Player Damage " << (sessions[vl]->GetCurrentPlayer()->GetStat().level * 5) << " <- " << sessions[vl]->GetCurrentPlayer()->GetName();
					string chat = ss.str();
					gamesession->ChatPkt(player->GetId(), chat);
				}
			}
		}
	}
}

void GameSessionManager::HandlePlayerDeath(GameSessionRef& _gamesession, PlayerRef& _player, uint64 _killerId)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _gamesession;
		player = _player;
	}

	for (int i = 0; i < MAX_USER; ++i)
	{
		READ_LOCK;
		if (sessions[i] != nullptr)
			sessions[i]->RemovePkt(player->GetId());
	}

	{
		player->GetOwnerSession()->s_lock.lock();
		player->SetState(ST_FREE);
		player->GetOwnerSession()->s_lock.unlock();
	}

	gamesession->ResetViewPlayer();
	ROOMMANAGER->LeaveRoom(gamesession);

	stringstream ss;
	ss << "Player " << sessions[_killerId]->GetCurrentPlayer()->GetName() << " Die ";
	string chat = ss.str();
	gamesession->ChatPkt(player->GetId(), chat);

	DoTimer(10, &GameSessionManager::PlayerRespawn, player->GetId());
}

void GameSessionManager::UpdateViewList(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl, int64 _movetime)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _gamesession;
		player = _player;
	}

	for (const auto& pl : _new_vl)
	{
		if (IsPlayer(pl) && sessions[pl]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
		{
			if (sessions[pl]->GetViewPlayer().count(player->GetId()))
				sessions[pl]->MovePkt(player->GetId(), player->GetPos(), _movetime);
			else {
				sessions[pl]->AddViewPlayer(player->GetId());
				sessions[pl]->AddObjectPkt(player->GetPT(), player->GetId(), player->GetName(), player->GetPos());
			}
		}

		if (_old_vl.count(pl) == 0 && player->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
		{
			gamesession->AddViewPlayer(pl);
			gamesession->AddObjectPkt(sessions[pl]->GetCurrentPlayer()->GetPT(), sessions[pl]->GetCurrentPlayer()->GetId(), sessions[pl]->GetCurrentPlayer()->GetName(), sessions[pl]->GetCurrentPlayer()->GetPos());
		}
	}

	for (const auto& pl : _old_vl)
	{
		if (_new_vl.count(pl) == 0)
		{
			if (player->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
				gamesession->RemovePkt(pl);
			gamesession->RemoveViewPlayer(pl);

			if (sessions[pl]->GetViewPlayer().count(player->GetId()))
			{
				if (sessions[pl]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
					sessions[pl]->RemovePkt(player->GetId());
				sessions[pl]->RemoveViewPlayer(player->GetId());
			}
		}
	}
}

void GameSessionManager::HandleNpcCollision(PlayerRef& _player, uint64 _pl)
{
	PlayerRef player;
	{
		player = _player;
	}

	{
		if (sessions[_pl]->GetCurrentPlayer()->POSX == _player->POSX && sessions[_pl]->GetCurrentPlayer()->POSY == _player->POSY)
		{

			int32 damage = sessions[_player->GetId()]->GetCurrentPlayer()->GetStat().level * 5;

			sessions[_pl]->s_lock.lock();
			sessions[_pl]->GetCurrentPlayer()->UpdateStatHp(damage * -1);
			sessions[_pl]->s_lock.unlock();

			sessions[_pl]->StatChangePkt(
				sessions[_pl]->GetCurrentPlayer()->GetStat().level,
				sessions[_pl]->GetCurrentPlayer()->GetStat().hp,
				sessions[_pl]->GetCurrentPlayer()->GetStat().maxHp,
				sessions[_pl]->GetCurrentPlayer()->GetStat().mp,
				sessions[_pl]->GetCurrentPlayer()->GetStat().maxMp,
				sessions[_pl]->GetCurrentPlayer()->GetStat().exp,
				sessions[_pl]->GetCurrentPlayer()->GetStat().maxExp
			);


			stringstream ss;
			ss << sessions[_pl]->GetCurrentPlayer()->GetName() << " <- Damage " << damage << " Attack ";
			string chat = ss.str();
			sessions[_pl]->ChatPkt(_pl, chat);

			if (sessions[_pl]->GetCurrentPlayer()->GetStat().hp <= 0)
			{
				{
					sessions[_pl]->s_lock.lock();
					sessions[_pl]->GetCurrentPlayer()->SetState(ST_FREE);
					sessions[_pl]->s_lock.unlock();
				}
				DoTimer(10, &GameSessionManager::PlayerRespawn, _pl);
			}
		}
	}
}

void GameSessionManager::HandleNPCViewListChanges(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _gamesession;
		player = _player;
	}

	for (auto pl : _new_vl)
	{
		if (_old_vl.count(pl) == 0)
		{
			POS pos = { player->POSX, player->POSY };
			sessions[pl]->AddObjectPkt(player->GetPT(), player->GetId(), player->GetName(), pos);
			sessions[pl]->AddViewPlayer(player->GetId());
		}
		else
		{
			auto now = std::chrono::system_clock::now();
			std::chrono::seconds since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
			std::int64_t posix_time = since_epoch.count();

			POS pos = { player->POSX, player->POSY };
			sessions[pl]->MovePkt(player->GetId(), pos, posix_time);

			if (player->GetId() > MAX_USER)
			{
				HandleNpcCollision(player, pl);
			}
		}
	}

	for (auto pl : _old_vl)
	{
		if (_new_vl.count(pl) == 0)
		{
			gamesession->RemoveViewPlayer(pl);

			if (sessions[pl])
			{
				if (sessions[pl]->GetViewPlayer().count(player->GetId()))
				{
					sessions[pl]->RemovePkt(player->GetId());
					sessions[pl]->RemoveViewPlayer(player->GetId());
				}
			}
		}
	}
}

void GameSessionManager::MoveNpcToRandomPosition(GameSessionRef& _gamesession, PlayerRef& _player)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		gamesession = _gamesession;
		player = _player;
	}

	std::vector<int> indices = gamesession->GetRandomDirectionIndices();

	for (int index : indices) {
		pair<int, int> pp = directions[index];
		int newX = player->POSX + pp.first;
		int newY = player->POSY + pp.second;

		if (newX >= 0 && newX < W_WIDTH && newY >= 0 && newY < W_HEIGHT && MAPDATA->GetTile(newY, newX) == MAPDATA->e_PLAT) {
			POS pos = { newX, newY };
			{
				player->GetOwnerSession()->s_lock.lock();
				player->SetPos(pos);
				player->GetOwnerSession()->s_lock.unlock();
			}
			break;
		}
	}
}

bool GameSessionManager::IsAdjacent(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];

	}
	return (target_gamesession->GetCurrentPlayer()->POSX == attack_gamesession->GetCurrentPlayer()->POSX && target_gamesession->GetCurrentPlayer()->POSY == attack_gamesession->GetCurrentPlayer()->POSY - 1) ||
		(target_gamesession->GetCurrentPlayer()->POSX == attack_gamesession->GetCurrentPlayer()->POSX && target_gamesession->GetCurrentPlayer()->POSY == attack_gamesession->GetCurrentPlayer()->POSY + 1) ||
		(target_gamesession->GetCurrentPlayer()->POSX == attack_gamesession->GetCurrentPlayer()->POSX - 1 && target_gamesession->GetCurrentPlayer()->POSY == attack_gamesession->GetCurrentPlayer()->POSY) ||
		(target_gamesession->GetCurrentPlayer()->POSX == attack_gamesession->GetCurrentPlayer()->POSX + 1 && target_gamesession->GetCurrentPlayer()->POSY == attack_gamesession->GetCurrentPlayer()->POSY);
}

void GameSessionManager::HandleAttack(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];

	}

	{
		if (target_gamesession->GetCurrentPlayer()->GetState() == ST_INGAME)
		{
			target_gamesession->s_lock.lock();
			target_gamesession->GetCurrentPlayer()->UpdateStatHp(attack_gamesession->GetCurrentPlayer()->GetStat().level * -3);
			target_gamesession->s_lock.unlock();

			if (target_gamesession->GetCurrentPlayer()->GetStat().hp <= 0) // Target died
				HandleNPCDeath(_attackerId, _targetId);
			else
				BroadcastAttackMessage(_attackerId, _targetId);
		}
	}
}

void GameSessionManager::HandleNPCDeath(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];

	}
	{
		target_gamesession->s_lock.lock();
		target_gamesession->GetCurrentPlayer()->active = false;
		target_gamesession->GetCurrentPlayer()->SetState(ST_FREE);
		target_gamesession->s_lock.unlock();
	}
	{
		attack_gamesession->s_lock.lock();
		attack_gamesession->GetCurrentPlayer()->UpdateStatExp(target_gamesession->GetCurrentPlayer()->GetStat().exp);
		attack_gamesession->s_lock.unlock();
	}
	{
		if (attack_gamesession->GetCurrentPlayer()->GetStat().exp >= attack_gamesession->GetCurrentPlayer()->GetStat().maxExp) // Level up
		{
			{
				attack_gamesession->s_lock.lock();
				attack_gamesession->GetCurrentPlayer()->LevelUp();
				attack_gamesession->s_lock.unlock();
			}
			attack_gamesession->StatChangePkt(
				attack_gamesession->GetCurrentPlayer()->GetStat().level,
				attack_gamesession->GetCurrentPlayer()->GetStat().hp,
				attack_gamesession->GetCurrentPlayer()->GetStat().maxHp,
				attack_gamesession->GetCurrentPlayer()->GetStat().mp,
				attack_gamesession->GetCurrentPlayer()->GetStat().maxMp,
				attack_gamesession->GetCurrentPlayer()->GetStat().exp,
				attack_gamesession->GetCurrentPlayer()->GetStat().maxExp
			);

			stringstream ss;
			ss << "Player LV " << (attack_gamesession->GetCurrentPlayer()->GetStat().level - 1) << " -> " << attack_gamesession->GetCurrentPlayer()->GetStat().level << " UP";
			string chat = ss.str();
			attack_gamesession->ChatPkt(_attackerId, chat);
		}
		else
		{
			stringstream ss;
			ss << "Player Catch " << (target_gamesession->GetCurrentPlayer()->GetName()) << " -> " << target_gamesession->GetCurrentPlayer()->GetStat().exp << " UP";
			string chat = ss.str();
			attack_gamesession->ChatPkt(_attackerId, chat);

			attack_gamesession->StatChangePkt(
				attack_gamesession->GetCurrentPlayer()->GetStat().level,
				attack_gamesession->GetCurrentPlayer()->GetStat().hp,
				attack_gamesession->GetCurrentPlayer()->GetStat().maxHp,
				attack_gamesession->GetCurrentPlayer()->GetStat().mp,
				attack_gamesession->GetCurrentPlayer()->GetStat().maxMp,
				attack_gamesession->GetCurrentPlayer()->GetStat().exp,
				attack_gamesession->GetCurrentPlayer()->GetStat().maxExp
			);
		}
	}
	for (int i = 0; i < MAX_USER; ++i)
	{
		READ_LOCK;
		if (sessions[i] != nullptr)
			sessions[i]->RemovePkt(_targetId);
	}

	{
		target_gamesession->s_lock.lock();
		target_gamesession->GetCurrentPlayer()->SetState(ST_SLEEP);
		target_gamesession->s_lock.unlock();
	}
	ROOMMANAGER->LeaveRoom(sessions[_targetId]);
	DoTimer(10000, &GameSessionManager::Respawn, target_gamesession);
}

void GameSessionManager::BroadcastAttackMessage(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];
	}
	stringstream ss;
	ss << "Player " << attack_gamesession->GetCurrentPlayer()->GetName() << " Attack -> " << target_gamesession->GetCurrentPlayer()->GetName();
	string chat = ss.str();
	attack_gamesession->ChatPkt(_attackerId, chat);

	unordered_set<uint64> viewList = attack_gamesession->GetViewPlayer();
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (i == _attackerId)
			continue;

		if (sessions[i] != nullptr && sessions[i]->GetCurrentPlayer() != nullptr)
		{
			for (auto viewerId : viewList)
			{
				sessions[viewerId]->ChatPkt(_attackerId, chat);
			}
		}
	}
}

void GameSessionManager::SetupPlayerAndSession(int _id, const string& _name, const STAT& _st, const POS& _pos, Protocol::PlayerType _pt)
{
	auto session = MakeShared<GameSession>();
	PlayerRef player = MakeShared<Player>(_name, _st, _pos, ST_INGAME, 9999, _pt);

	{
		player->SetId(_id);
		session->SetId(_id);
		session->SetCurrentPlayer(player);
		player->SetOwnerSession(session);
	}

	{
		sessions[_id] = session;
	}
	ROOMMANAGER->EnterRoom(session);
}

void GameSessionManager::GenerateNPCAttributes(int _id, string& _name, STAT& _st, POS& _pos, Protocol::PlayerType& _pt)
{
	stringstream ss;
	ss << "NPC " << _id;
	_name = ss.str();

	while (true) {
		int x = rand() % 2000;
		int y = rand() % 2000;

		if (MAPDATA->GetTile(y, x) == MAPDATA->e_PLAT) {
			_pos = { x, y };
			break;
		}
	}

	if (_id >= MAX_USER && _id < MAX_USER + MAX_NPC / 4) {
		_st = { LV1STAT };
		_pt = Protocol::PLAYER_TYPE_LV1;
	}
	else if (_id >= MAX_USER + MAX_NPC / 4 && _id < MAX_USER + (MAX_NPC / 4 * 2)) {
		_st = { LV2STAT };
		_pt = Protocol::PLAYER_TYPE_LV2;
	}
	else if (_id >= MAX_USER + (MAX_NPC / 4 * 2) && _id < MAX_USER + (MAX_NPC / 4 * 3)) {
		_st = { LV3STAT };
		_pt = Protocol::PLAYER_TYPE_LV3;
	}
	else {
		_st = { LV4STAT };
		_pt = Protocol::PLAYER_TYPE_LV4;
	}

}

GameSessionRef GameSessionManager::GetSession(uint64 _id)
{
	{
		READ_LOCK;
		if (sessions[_id] != nullptr)
			return sessions[_id];
		else
			return nullptr;
	}
}

void GameSessionManager::InitializeNPC()
{
	cout << "NPC initialize begin." << endl;

	for (int i = MAX_USER + 1; i < MAX_USER + MAX_NPC; ++i) {
		string name;
		STAT st;
		POS pos;
		Protocol::PlayerType pt;

		GenerateNPCAttributes(i, name, st, pos, pt);
		SetupPlayerAndSession(i, name, st, pos, pt);
	}


	cout << "NPC initialize end." << endl;
}


