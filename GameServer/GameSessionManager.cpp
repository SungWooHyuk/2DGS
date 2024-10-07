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
	WRITE_LOCK;
	int id = _session->GetCurrentPlayer()->GetId();

	if (_session->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
		ASSERT_CRASH(SaveDBPlayer(id));

	unordered_set<uint64> vl = sessions[id]->GetViewPlayer(); // Lock 체크
	for (const auto& _vl : vl)
	{
		if (false == IsPlayer(_vl)) continue;
		if (_vl == id) continue;
		if (PLAYER(_vl)->GetState() != ST_INGAME) continue;
		if (sessions[_vl]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_DUMMY) continue;

		sessions[_vl]->RemovePkt(id);
		sessions[_vl]->RemoveViewPlayer(id);
	}

	freeId.push(id);
	sessions[id]->GetCurrentPlayer()->SetState(ST_FREE);
	sessions[id].reset();
}

void GameSessionManager::Broadcast(SendBufferRef _sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : sessions)
	{
		if (session->GetCurrentPlayer()->GetId() < MAX_USER)
			session->Send(_sendBuffer);
	}
}

void GameSessionManager::Respawn(GameSessionRef _session)
{
	GameSessionRef gamesession = _session;
	PlayerRef player = gamesession->GetCurrentPlayer();
	ROOMMANAGER->EnterRoom(_session); // 룸 다시 입장.

	player->SetStatHp(player->GetStat().maxHp);

	unordered_set<uint64> vl = ROOMMANAGER->ViewList(gamesession, true);
	{
		WRITE_LOCK;
		gamesession->SetViewPlayer(vl);
	}
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
		WRITE_LOCK;
		_session->GetCurrentPlayer()->SetState(ST_INGAME);
	}
}

void GameSessionManager::PlayerRespawn(uint64 _id)
{
	POS pos{ TOWN };
	{
		WRITE_LOCK;
		PLAYER(_id)->SetStatHp(PLSTAT(_id).maxHp);
		PLAYER(_id)->SetPos(pos);
		PLAYER(_id)->SetStatExp(PLSTAT(_id).exp / 2);
		PLAYER(_id)->SetState(ST_INGAME);
	}
	ROOMMANAGER->EnterRoom(sessions[_id]);
	sessions[_id]->RespawnPkt(PLSTAT(_id).hp, pos, PLSTAT(_id).exp);
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
	PlayerRef player = _player;
	PlayerRef target = _toPlayer;
	unordered_set<uint64> vl = ROOMMANAGER->ViewList(player->GetOwnerSession(), true);
	{
		WRITE_LOCK;
		player->GetOwnerSession()->SetViewPlayer(vl);
	}

	if (player->GetPT() == Protocol::PLAYER_TYPE_CLIENT) return;
	if (player->active) return; // 이미 깨어있으면 return
	bool old_state = false;
	if (false == atomic_compare_exchange_strong(&player->active, &old_state, true))
		return; // 한쓰레드만 접근하게 
	DoTimer(1000, &GameSessionManager::NpcAstarMove, player->GetId());
}

void GameSessionManager::NpcRandomMove(uint64 _id)
{
	PlayerRef npc;
	{
		READ_LOCK;
		npc = sessions[_id]->GetCurrentPlayer();
	}

	if (sessions[_id]->GetViewPlayer().size() > 0) {
		if (sessions[_id]->GetCurrentPlayer()->GetState() == ST_INGAME)
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
	{
		READ_LOCK;
		npc = sessions[_id]->GetCurrentPlayer();
	}

	unordered_set<uint64> vl = ROOMMANAGER->ViewList(sessions[_id]);
	sessions[_id]->SetViewPlayer(vl);

	unordered_set<uint64> viewPlayer = sessions[_id]->GetViewPlayer();
	uint32 closestDistance = INT32_MAX;
	uint32 closestPlayerId = 0;

	if (viewPlayer.size() > 0) {
		if (sessions[_id]->GetCurrentPlayer()->GetState() == ST_INGAME) // 굳이?
		{
	
			if (sessions[_id]->EmptyPath() || sessions[_id]->GetPathCount() > 4)
			{
				sessions[_id]->ResetPath();
				for (auto id : viewPlayer)
				{
					if (sessions[id]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT && sessions[id]->GetCurrentPlayer()->GetState() == ST_INGAME)
					{
						POS playerPos = sessions[id]->GetCurrentPlayer()->GetPos();

						uint32 distance = abs(sessions[_id]->GetCurrentPlayer()->GetPos().posx - playerPos.posx) + abs(sessions[_id]->GetCurrentPlayer()->GetPos().posy - playerPos.posy);


						if (distance < closestDistance) {
							closestDistance = distance;
							closestPlayerId = id;
						}
					}
				}
			}

			NpcAstar(sessions[_id]->GetCurrentPlayer()->GetId(), closestPlayerId);
			DoNpcAstarMove(_id);
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

bool GameSessionManager::DoNpcRandomMove(uint64 _id)
{
	GameSessionRef gamesession = sessions[_id];
	PlayerRef player = gamesession->GetCurrentPlayer();
	unordered_set<uint64> old_vl;
	{
		READ_LOCK;
		old_vl = gamesession->GetViewPlayer();
	}

	MoveNpcToRandomPosition(gamesession, player);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession, true);

	HandleNPCViewListChanges(gamesession, player, old_vl, new_vl);

	return true;
}

bool GameSessionManager::DoNpcAstarMove(uint64 _id)
{
	GameSessionRef gamesession = sessions[_id];
	PlayerRef player = gamesession->GetCurrentPlayer();
	unordered_set<uint64> old_vl;
	{
		READ_LOCK;
		old_vl = gamesession->GetViewPlayer();
	}

	AstarMove(gamesession);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession, true);

	HandleNPCViewListChanges(gamesession, player, old_vl, new_vl);

	return true;
}

void GameSessionManager::AstarMove(GameSessionRef& _session)
{
	GameSessionRef gamesession = _session;
	int index = gamesession->GetPathIndex();
	vector<POS> v = gamesession->GetPath();
	int size = v.size();
	
	gamesession->SetPathCount(gamesession->GetPathCount() + 1);

	if (!v.empty())
	{
		if (index >= size)
		{
			{
				WRITE_LOCK;
				gamesession->GetCurrentPlayer()->SetPos(v[index - 1]);
			}
		}
		else
		{
			{
				WRITE_LOCK;
				gamesession->GetCurrentPlayer()->SetPos(v[index]);
			}

			gamesession->SetPathIndex(index + 1);
		}
	}
}

void GameSessionManager::NpcAstar(uint64 _id, uint64 _destId)
{
	GameSessionRef gamesession = sessions[_id];
	PlayerRef player = gamesession->GetCurrentPlayer();

	if (gamesession->EmptyPath()) // 비어있을때만 진행해준다. 즉, 현재 경로가없으면.
	{
		// 점수 매기기
	// F = G+H
	// F = 최종점수 ( 작을 수록 좋고, 경로에 따라 달라짐 )
	// G = 시작점에서 해당 좌표까지 이동하는데 드는 비용 (작을수록 좋고, 경로에 따라 달라짐)
	// H = 목적지에서 얼마나 가까운지 ( 작을 수록 좋음, 고정 )
		

		POS start = player->GetPos(); // NPC의 현재위치 start
		POS dest;
		if (sessions[_destId])
			dest = sessions[_destId]->GetCurrentPlayer()->GetPos();// 나를 깨운 player
		else
			return;


		//방문 여부
		vector<vector<bool>> closed(W_HEIGHT, vector<bool>(W_WIDTH, false));
		//최고값
		vector<vector<int>> best(W_HEIGHT, vector<int>(W_WIDTH, INT32_MAX));
		//부모 추적
		map<POS, POS> parent;
		//open list -> 이거 concurrent 쓸지말지 고민 - 쓰레드세이프한거
		priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;
		// 1. 예약(발견) 시스템 구현
		// - 여기서 이미 더 좋은 경로를 찾았다면 스킵
		// 2. 뒤늦게 더 좋은 경로가 발견될시에는 예외처리 필수적으로
		// - openlist에서 찾아서 제거하거나
		// - pq에서 pop한 다음에 무시한다거나

		// 초기값
		{
			int32 g = 0;
			int32 h = 10 * (abs(dest.posy - start.posy) + abs(dest.posx - start.posx));
			pq.push(PQNode{ g + h, g, start }); // 초기값 넣기
			best[start.posy][start.posx] = g + h; // 당연한것
			parent[start] = start; // 초기값의 부모는 나
		}

		while (pq.empty() == false)
		{
			// 제일 좋은 후보를 찾아준다.
			PQNode node = pq.top();
			pq.pop();

			// 동일한 좌표를 여러 경로로 찾을 예정이다.
			// 거기서 더 빠른 경로로 인해서 이미 방문 즉, 닫힌 경우엔 스킵
			// 선택
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

				// 자 이제 계산
				int32 g = node.g + cost[dir];
				int32 h = 10 * (abs(dest.posy - nextPos.posy) + abs(dest.posx - nextPos.posx));

				// 다른 경로에서 더 빠른 길을 찾았으면 스킵
				if (best[nextPos.posy][nextPos.posx] <= g + h)
					continue; // 베스트가 더 적다. 즉 이 길이 아니다.

				// 오 여기가 제일 빠르네
				best[nextPos.posy][nextPos.posx] = g + h;
				pq.push(PQNode{ g + h, g, nextPos });
				parent[nextPos] = node.pos;
			}
		}

		gamesession->SetPath(dest, parent); // 이제 gamesession의 path에 내가 가야하는 길들이 저장된다.
	}
	else
		gamesession->SetPathCount(gamesession->GetPathCount() + 1);
}

void GameSessionManager::Attack(GameSessionRef& _session, uint64 _id, uint64 _skill)
{
	unordered_set<uint64> vl = _session->GetViewPlayer();

	for (const auto& _vl : vl)
	{
		if (IsPlayer(_vl)) // Player ignore
			continue;

		if (IsAdjacent(_id, _vl))
		{
			HandleAttack(_id, _vl);
		}
	}

}

void GameSessionManager::Move(GameSessionRef& _session, uint64 _direction, int64 _movetime)
{
	GameSessionRef gamesession = _session;
	PlayerRef player = gamesession->GetCurrentPlayer();
	unordered_set<uint64> old_vl;

	{
		READ_LOCK;
		old_vl = gamesession->GetViewPlayer();
	}
	UpdatePlayerPosition(player, _direction);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession); // 움직인 후 주변애들 + WakeNPC까지 진행완료

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
	SAVEDB db;
	db.level = sessions[_id]->GetCurrentPlayer()->GetStat().level;
	db.hp = sessions[_id]->GetCurrentPlayer()->GetStat().hp;
	db.maxHp = sessions[_id]->GetCurrentPlayer()->GetStat().maxHp;
	db.mp = sessions[_id]->GetCurrentPlayer()->GetStat().mp;
	db.maxMp = sessions[_id]->GetCurrentPlayer()->GetStat().maxMp;
	db.exp = sessions[_id]->GetCurrentPlayer()->GetStat().exp;
	db.maxExp = sessions[_id]->GetCurrentPlayer()->GetStat().maxExp;
	db.name = sessions[_id]->GetCurrentPlayer()->GetName();
	db.posx = sessions[_id]->GetCurrentPlayer()->GetPos().posx;
	db.posy = sessions[_id]->GetCurrentPlayer()->GetPos().posy;

	return DB->SaveDB(db);
}

void GameSessionManager::UpdatePlayerPosition(PlayerRef& _player, uint64 _direction)
{
	PlayerRef player = _player;
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
		WRITE_LOCK;
		player->SetPos(pos);
	}
}

void GameSessionManager::HandleCollisions(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _new_vl)
{
	for (const auto& vl : _new_vl)
	{
		if (PLAYER(vl)->GetPT() != Protocol::PLAYER_TYPE_CLIENT && PLAYER(vl)->GetPT() != Protocol::PLAYER_TYPE_DUMMY)
		{
			if (PLAYER(vl)->GetPos() == _player->GetPos())
			{
				{
					WRITE_LOCK;
					_player->UpdateStatHp(PLSTAT(vl).level * -5);
				}

				if (_player->GetStat().hp <= 0)
				{
					HandlePlayerDeath(_gamesession, _player, vl);
					return;
				}
				else
				{
					_gamesession->StatChangePkt(
						_player->GetStat().level,
						_player->GetStat().hp,
						_player->GetStat().maxHp,
						_player->GetStat().mp,
						_player->GetStat().maxMp,
						_player->GetStat().exp,
						_player->GetStat().maxExp
					);

					stringstream ss;
					ss << "Player Damage " << (PLSTAT(vl).level * 5) << " <- " << PLAYER(vl)->GetName();
					string chat = ss.str();
					_gamesession->ChatPkt(_player->GetId(), chat);
				}
			}
		}
	}
}

void GameSessionManager::HandlePlayerDeath(GameSessionRef& _gamesession, PlayerRef& _player, uint64 _killerId)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (sessions[i] != nullptr)
			sessions[i]->RemovePkt(_player->GetId());
	}

	{
		WRITE_LOCK;
		_player->SetState(ST_FREE);
	}

	_gamesession->ResetViewPlayer();
	ROOMMANAGER->LeaveRoom(_gamesession);

	stringstream ss;
	ss << "Player " << PLAYER(_killerId)->GetName() << " Die ";
	string chat = ss.str();
	_gamesession->ChatPkt(_player->GetId(), chat);
	
	DoTimer(10, &GameSessionManager::PlayerRespawn, _player->GetId());
}

void GameSessionManager::UpdateViewList(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl, int64 _movetime)
{
	for (const auto& pl : _new_vl)
	{
		if (IsPlayer(pl) && sessions[pl]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
		{
			if (sessions[pl]->GetViewPlayer().count(_player->GetId()))
				sessions[pl]->MovePkt(_player->GetId(), _player->GetPos(), _movetime);
			else {
				sessions[pl]->AddViewPlayer(_player->GetId());
				sessions[pl]->AddObjectPkt(_player->GetPT(), _player->GetId(), _player->GetName(), _player->GetPos());
			}
		}

		if (_old_vl.count(pl) == 0 && _player->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
		{
			_gamesession->AddViewPlayer(pl);
			_gamesession->AddObjectPkt(PLAYER(pl)->GetPT(), PLAYER(pl)->GetId(), PLAYER(pl)->GetName(), PLAYER(pl)->GetPos());
		}
	}

	for (const auto& pl : _old_vl)
	{
		if (_new_vl.count(pl) == 0)
		{
			if (_player->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
				_gamesession->RemovePkt(pl);
			_gamesession->RemoveViewPlayer(pl);

			if (sessions[pl]->GetViewPlayer().count(_player->GetId()))
			{
				if (sessions[pl]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
					sessions[pl]->RemovePkt(_player->GetId());
				sessions[pl]->RemoveViewPlayer(_player->GetId());
			}
		}
	}
}

void GameSessionManager::HandleNpcCollision(PlayerRef& _player, uint64 _pl)
{
	if (PLAYER(_pl)->POSX == _player->POSX && PLAYER(_pl)->POSY == _player->POSY)
	{
		int32 damage = PLSTAT(_player->GetId()).level * 5;
		PLAYER(_pl)->UpdateStatHp(damage * -1);
		
		sessions[_pl]->StatChangePkt(
			PLAYER(_pl)->GetStat().level,
			PLAYER(_pl)->GetStat().hp,
			PLAYER(_pl)->GetStat().maxHp,
			PLAYER(_pl)->GetStat().mp,
			PLAYER(_pl)->GetStat().maxMp,
			PLAYER(_pl)->GetStat().exp,
			PLAYER(_pl)->GetStat().maxExp
		);


		stringstream ss;
		ss << PLAYER(_pl)->GetName() << " <- Damage " << damage << " Attack ";
		string chat = ss.str();
		sessions[_pl]->ChatPkt(_pl, chat);

		if (PLAYER(_pl)->GetStat().hp <= 0)
		{
			{
				WRITE_LOCK;
				PLAYER(_pl)->SetState(ST_FREE);
			}
			DoTimer(10, &GameSessionManager::PlayerRespawn, _pl);
		}
	}
}

void GameSessionManager::HandleNPCViewListChanges(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl)
{
	for (auto pl : _new_vl)
	{
		if (_old_vl.count(pl) == 0)
		{
			POS pos = { _player->POSX, _player->POSY };
			sessions[pl]->AddObjectPkt(_player->GetPT(), _player->GetId(), _player->GetName(), pos);
		}
		else
		{
			auto now = std::chrono::system_clock::now();
			std::chrono::seconds since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
			std::int64_t posix_time = since_epoch.count();

			POS pos = { _player->POSX, _player->POSY };
			sessions[pl]->MovePkt(_player->GetId(), pos, posix_time);

			if (_player->GetId() > MAX_USER)
			{
				HandleNpcCollision(_player, pl);
			}
		}
	}

	for (auto pl : _old_vl)
	{
		if (_new_vl.count(pl) == 0)
		{
			_gamesession->RemoveViewPlayer(pl);
			if (sessions[pl]->GetViewPlayer().count(_player->GetId()))
			{
				sessions[pl]->RemovePkt(_player->GetId());
				sessions[pl]->RemoveViewPlayer(_player->GetId());
			}
		}
	}
}

void GameSessionManager::MoveNpcToRandomPosition(GameSessionRef& _gamesession, PlayerRef& _player)
{
	std::vector<int> indices = _gamesession->GetRandomDirectionIndices();

	for (int index : indices) {
		pair<int, int> pp = directions[index];
		int newX = _player->POSX + pp.first;
		int newY = _player->POSY + pp.second;

		if (newX >= 0 && newX < W_WIDTH && newY >= 0 && newY < W_HEIGHT && MAPDATA->GetTile(newY, newX) == MAPDATA->e_PLAT) {
			POS pos = { newX, newY };
			{
				WRITE_LOCK;
				_player->SetPos(pos);
			}
			break;
		}
	}
}

bool GameSessionManager::IsAdjacent(uint64 _attackerId, uint64 _targetId)
{
	return (PLAYER(_targetId)->POSX == PLAYER(_attackerId)->POSX && PLAYER(_targetId)->POSY == PLAYER(_attackerId)->POSY - 1) ||
		(PLAYER(_targetId)->POSX == PLAYER(_attackerId)->POSX && PLAYER(_targetId)->POSY == PLAYER(_attackerId)->POSY + 1) ||
		(PLAYER(_targetId)->POSX == PLAYER(_attackerId)->POSX - 1 && PLAYER(_targetId)->POSY == PLAYER(_attackerId)->POSY) ||
		(PLAYER(_targetId)->POSX == PLAYER(_attackerId)->POSX + 1 && PLAYER(_targetId)->POSY == PLAYER(_attackerId)->POSY);
}

void GameSessionManager::HandleAttack(uint64 _attackerId, uint64 _targetId)
{
	{
		WRITE_LOCK;
		PLAYER(_targetId)->UpdateStatHp(PLSTAT(_attackerId).level * -3);
	}

	if (PLSTAT(_targetId).hp <= 0) // Target died
		HandleNPCDeath(_attackerId, _targetId);
	else
		BroadcastAttackMessage(_attackerId, _targetId);

}

void GameSessionManager::HandleNPCDeath(uint64 _attackerId, uint64 _targetId)
{
	{
		WRITE_LOCK;
		PLAYER(_targetId)->active = false;
		PLAYER(_targetId)->SetState(ST_FREE);
		PLAYER(_attackerId)->UpdateStatExp(PLSTAT(_targetId).exp);
	}

	if (PLSTAT(_attackerId).exp >= PLSTAT(_attackerId).level * 100) // Level up
	{
		PLAYER(_attackerId)->LevelUp();
		sessions[_attackerId]->StatChangePkt(
			PLSTAT(_attackerId).level, PLSTAT(_attackerId).hp, PLSTAT(_attackerId).maxHp,
			PLSTAT(_attackerId).mp, PLSTAT(_attackerId).maxMp, PLSTAT(_attackerId).exp, PLSTAT(_attackerId).maxExp
		);

		stringstream ss;
		ss << "Player LV " << (PLSTAT(_attackerId).level - 1) << " -> " << PLSTAT(_attackerId).level << " UP";
		string chat = ss.str();
		sessions[_attackerId]->ChatPkt(_attackerId, chat);
	}
	else
	{
		stringstream ss;
		ss << "Player Catch " << (sessions[_targetId]->GetCurrentPlayer()->GetName()) << " -> " << PLSTAT(_targetId).exp << " UP";
		string chat = ss.str();
		sessions[_attackerId]->ChatPkt(_attackerId, chat);

		sessions[_attackerId]->StatChangePkt(
			PLSTAT(_attackerId).level, PLSTAT(_attackerId).hp, PLSTAT(_attackerId).maxHp,
			PLSTAT(_attackerId).mp, PLSTAT(_attackerId).maxMp, PLSTAT(_attackerId).exp, PLSTAT(_attackerId).maxExp
		);
	}

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (sessions[i] != nullptr)
			sessions[i]->RemovePkt(_targetId); // Notify all players about removal
	}

	{
		WRITE_LOCK;
		PLAYER(_targetId)->SetState(ST_SLEEP);
	}
	ROOMMANAGER->LeaveRoom(sessions[_targetId]);
	DoTimer(10000, &GameSessionManager::Respawn, sessions[_targetId]);
}

void GameSessionManager::BroadcastAttackMessage(uint64 _attackerId, uint64 _targetId)
{
	stringstream ss;
	ss << "Player " << PLAYER(_attackerId)->GetName() << " Attack -> " << PLAYER(_targetId)->GetName();
	string chat = ss.str();
	sessions[_attackerId]->ChatPkt(_attackerId, chat);

	unordered_set<uint64> viewList = sessions[_attackerId]->GetViewPlayer();
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
		WRITE_LOCK;
		player->SetId(_id);
		session->SetId(_id);
		session->SetCurrentPlayer(player);
		player->SetOwnerSession(session);
	}

	sessions[_id] = session;
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
	if (sessions[_id] != nullptr)
		return sessions[_id];
	else
		return nullptr;
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


