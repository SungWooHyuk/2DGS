#include "pch.h"
#include "utils.h"
#include <sstream>
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Player.h"
#include "MapData.h"
#include "RoomManager.h"
#include "RedisManager.h"
#include "DropTable.h"
#include "item.h"
#include "Protocol.pb.h"
#include <fstream>
#include "GLogger.h"

shared_ptr<GameSessionManager> GGameSessionManager = make_shared<GameSessionManager>();

void GameSessionManager::Add(GameSessionRef _session)
{

	int newId = GetNewClientId();
	{
		WRITE_LOCK;
		_session->SetId(newId);
		sessions[newId] = _session;
	}

	GLogger::LogWithContext(spdlog::level::info, "GameSessionManager", "Add", "New session added with ID: {}", newId);

}

void GameSessionManager::Remove(GameSessionRef _session)
{
	if (!_session)
	{
		ASSERT_CRASH(false);
		return;
	}

	GameSessionRef gamesession = _session;
	auto currentPlayer = gamesession->GetCurrentPlayer();

	if (!currentPlayer) {
		GLogger::Log(spdlog::level::err, "[GameSessionManager][Remove] currentplayer already remove");
		return;
	}

	int id = currentPlayer->GetId();
	GLogger::LogWithContext(spdlog::level::info, currentPlayer->GetName(), "Remove", "Removing session for player ID: {}", id);

	if (currentPlayer->GetPT() == Protocol::PLAYER_TYPE_CLIENT) {
		if (currentPlayer->GetName().substr(0, 5) != "dummy")
		{
			DBMANAGER.SendSavePkt(
				id,
				currentPlayer->GetInventory(),
				currentPlayer->GetEquipments(),
				currentPlayer->GetPlayer()
			);
		}
	}

	unordered_set<uint64> vl;
	{
		READ_LOCK;
		if (sessions[id])
			vl = sessions[id]->GetViewPlayer();
	}

	{
		READ_LOCK;
		for (const auto& _vl : vl)
		{
			if (!IsPlayer(_vl)) continue;
			if (_vl == id) continue;

			auto player = sessions[_vl]->GetCurrentPlayer();
			if (!player) continue;

			if (player->GetState() != ST_INGAME) continue;
			if (player->GetPT() == Protocol::PLAYER_TYPE_DUMMY) continue;

			sessions[_vl]->RemovePkt(id);
			sessions[_vl]->RemoveViewPlayer(id);
		}
	}

	{
		WRITE_LOCK;
		if (sessions[id])
		{
			sessions[id]->GetCurrentPlayer()->SetState(ST_FREE);
			freeId.push(id);
			sessions[id].reset();
		}
	}
}

void GameSessionManager::Broadcast(SendBufferRef _sendBuffer)
{
	READ_LOCK;
	for (auto session : sessions)
	{
		if (session && session->GetCurrentPlayer()->GetId() < MAX_USER)
			session->Send(_sendBuffer);
	}
}

void GameSessionManager::SessionRankingUpdate()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		READ_LOCK;
		if (sessions[i] != nullptr)
			sessions[i]->RankingPkt(REDIS.GetTop5Ranking());
	}
}

void GameSessionManager::Respawn(GameSessionRef _session)
{
	GameSessionRef gamesession = _session;
	PlayerRef player = gamesession->GetCurrentPlayer();

	
	ROOMMANAGER->EnterRoom(_session);

	player->SetStatHp(player->GetStat().maxHp);

	unordered_set<uint64> vl = ROOMMANAGER->ViewList(gamesession, true);
	gamesession->SetViewPlayer(vl);

	{
		READ_LOCK;
		for (auto _vl : vl)
		{
			POS plPos = { player->POSX, player->POSY };

			if (sessions[_vl])
			{
				sessions[_vl]->AddObjectPkt(
					player->GetPT(),
					player->GetId(),
					player->GetName(),
					plPos
				);
			}
		}
	}


	player->SetState(ST_INGAME);
}

void GameSessionManager::PlayerRespawn(uint64 _id)
{
	POS pos{ TOWN };
	GameSessionRef session;
	PlayerRef player;
	GLogger::Log(spdlog::level::info, player->GetName(), "Respawn");

	{
		READ_LOCK;
		session = sessions[_id];
	}

	if (!session) {
		GLogger::Log(spdlog::level::err, "[GameSessionManager][PlayerRespawn] !session");
		return;
	}
	player = session->GetCurrentPlayer();

	player->SetStatHp(player->GetStat().maxHp);
	player->SetPos(pos);
	player->SetStatExp(player->GetStat().exp / 2);
	player->SetState(ST_INGAME);

	ROOMMANAGER->EnterRoom(session);
	session->RespawnPkt(player->GetStat().hp, pos, player->GetStat().exp);
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

	READ_LOCK;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (sessions[i] == nullptr)
			return i;
	}

	GLogger::Log(spdlog::level::err, "[GameSessionManager][GetNewClientId] -1");
	return -1;
}

void GameSessionManager::WakeNpc(PlayerRef _player, PlayerRef _toPlayer)
{
	PlayerRef player = _player;
	PlayerRef target = _toPlayer;

	unordered_set<uint64> vl = ROOMMANAGER->ViewList(player->GetOwnerSession(), true);
	player->GetOwnerSession()->SetViewPlayer(vl);

	if (player->GetPT() == Protocol::PLAYER_TYPE_CLIENT) return;
	if (player->active) return; 
	bool old_state = false;
	if (false == atomic_compare_exchange_strong(&player->active, &old_state, true))
		return; 
	DoTimer(1000, &GameSessionManager::NpcAstarMove, player->GetId());
}

void GameSessionManager::NpcRandomMove(uint64 _id)
{
	GameSessionRef gamesession;
	{
		READ_LOCK;
		if (sessions[_id])
			gamesession = sessions[_id];
	} 
	PlayerRef npc = gamesession->GetCurrentPlayer();

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
	GameSessionRef gamesession;
	PlayerRef npc;
	{
		READ_LOCK;
		if (sessions[_id])
			gamesession = sessions[_id];
	}

	if (!gamesession) return;
	npc = gamesession->GetCurrentPlayer();
	if (!npc) return;

	unordered_set<uint64> viewPlayer = gamesession->GetViewPlayer();
	if (viewPlayer.empty() || npc->GetState() != ST_INGAME)
	{
		npc->active = false;
		gamesession->ResetPath();
		return;
	}

	uint32 closestDistance = INT32_MAX;
	uint64 closestPlayerId = 0;

	if (gamesession->EmptyPath() || gamesession->GetPathCount() > 3)
	{
		gamesession->ResetPath();

		{
			READ_LOCK;
			for (auto id : viewPlayer)
			{
				auto session = sessions[id];
				if (!session) continue;

				PlayerRef player = session->GetCurrentPlayer();
				if (!player) continue;

				if (player->GetPT() == Protocol::PLAYER_TYPE_CLIENT && player->GetState() == ST_INGAME)
				{
					POS playerPos = player->GetPos();
					uint32 distance = abs(npc->GetPos().posx - playerPos.posx) + abs(npc->GetPos().posy - playerPos.posy);
					if (distance < closestDistance)
					{
						closestDistance = distance;
						closestPlayerId = id;
					}
				}
			}
		}

		NpcAstar(npc->GetId(), closestPlayerId);
	}

	if (DoNpcAstarMove(_id))
		DoTimer(1000, &GameSessionManager::NpcAstarMove, _id);
}

void GameSessionManager::NpcAstarMoveTo(uint64 _id, uint64 _targetid)
{
	PlayerRef npc;
	GameSessionRef gamesession;
	{
		READ_LOCK;
		gamesession = sessions[_id];
		npc = gamesession->GetCurrentPlayer();
	}

	unordered_set<uint64> viewPlayer = gamesession->GetViewPlayer();
	uint32 closestDistance = INT32_MAX;
	uint32 closestPlayerId = 0;

	if (viewPlayer.size() > 0) {
		if (npc->GetState() == ST_INGAME)
		{

			NpcAstar(npc->GetId(), _targetid);

			if (DoNpcAstarMove(_id))
				DoTimer(1000, &GameSessionManager::NpcAstarMoveTo, _id, _targetid);
		}
		else {
			npc->active = false;
			gamesession->ResetPath();
		}
	}
	else {
		npc->active = false;
		gamesession->ResetPath();
	}
}

bool GameSessionManager::DoNpcRandomMove(uint64 _id)
{
	GameSessionRef gamesession;
	PlayerRef player;
	{
		READ_LOCK;
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
		READ_LOCK;
		if (!sessions[_id]) {
			GLogger::Log(spdlog::level::err, "[GameSessionManager][DoNpcAstarMove] !sessions[_id]");
			return false;
		}

		gamesession = sessions[_id];
		player = gamesession->GetCurrentPlayer();
		if (!player) {
			GLogger::Log(spdlog::level::err, "[GameSessionManager][DoNpcAstarMove] !player");
			return false;
		}
	}

	unordered_set<uint64> old_vl = gamesession->GetViewPlayer();

	AstarMove(gamesession, player);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession, true);

	HandleNPCViewListChanges(gamesession, player, old_vl, new_vl);

	return true;
}

void GameSessionManager::AstarMove(GameSessionRef& _session, PlayerRef& _player)
{
	GameSessionRef gamesession = _session;
	PlayerRef player = _player;

	int index = gamesession->GetPathIndex();
	const vector<POS>& path = gamesession->GetPath();
	int size = path.size();

	if (size == 0)
		return;

	if (index >= size)
		return;

	player->SetPos(path[index]);
	gamesession->SetPathIndex(index + 1);
	gamesession->SetPathCount(gamesession->GetPathCount() + 1);
}

void GameSessionManager::NpcAstar(uint64 _id, uint64 _destId)
{
	GameSessionRef gamesession;
	GameSessionRef dest_gamesession;
	{
		READ_LOCK;
		gamesession = sessions[_id];
		dest_gamesession = sessions[_destId];
	}

	if (!gamesession || !dest_gamesession) {
		GLogger::Log(spdlog::level::err, "[GameSessionManager][NpcAstar] gamesession: {}, dest_gamesession: {} !gamesession || !dest_gamesession",
			_id, _destId);
		return;
	}
	PlayerRef player = gamesession->GetCurrentPlayer();
	PlayerRef target = dest_gamesession->GetCurrentPlayer();
	if (!player || !target) {
		GLogger::Log(spdlog::level::err, "[GameSessionManager][NpcAstar] !player || !target");
		return;
	}
	if (!gamesession->EmptyPath()) {
		return;
	}
	POS start = player->GetPos();
	POS dest = target->GetPos();
	if (start == dest) return;

	const int MAX_ITERATIONS = 1000;

	vector<vector<bool>> closed(W_HEIGHT, vector<bool>(W_WIDTH, false));
	vector<vector<int>> best(W_HEIGHT, vector<int>(W_WIDTH, INT32_MAX));
	map<POS, POS> parent;
	priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;

	pq.push(PQNode{ 0, 0, start });
	best[start.posy][start.posx] = 0;
	parent[start] = start;

	int cnt = 0;
	while (!pq.empty() && cnt < MAX_ITERATIONS)
	{
		PQNode node = pq.top();
		pq.pop();

		if (closed[node.pos.posy][node.pos.posx]) continue;
		closed[node.pos.posy][node.pos.posx] = true;

		if (node.pos == dest)
		{
			gamesession->SetPath(dest, parent);
			return;
		}

		for (int32 dir = 0; dir < DIR_COUNT; dir++)
		{
			POS nextPos = node.pos + dirs[dir];
			if (!gamesession->CanGo(nextPos) || closed[nextPos.posy][nextPos.posx]) continue;

			int32 g = node.g + cost[dir];
			int32 h = 10 * (abs(dest.posy - nextPos.posy) + abs(dest.posx - nextPos.posx));
			int32 f = g + h;

			if (best[nextPos.posy][nextPos.posx] <= f) continue;

			best[nextPos.posy][nextPos.posx] = f;
			pq.push(PQNode{ f, g, nextPos });
			parent[nextPos] = node.pos;
		}
		cnt++;
	}

	if (cnt >= MAX_ITERATIONS)
	{
		GLogger::LogWithContext(spdlog::level::err, "Astar", "MAX_ITERATIONS", "NPC Id: {}", _id);
	}
}

void GameSessionManager::Attack(GameSessionRef& _session, uint64 _id, uint64 _skill)
{
	GameSessionRef gamesession = _session;
	unordered_set<uint64> vl = gamesession->GetViewPlayer();

	for (const auto& _vl : vl)
	{
		if (IsPlayer(_vl)) // Player ignore
			continue;

		if (IsAdjacent(_id, _vl))
			HandleAttack(_id, _vl);
	}

}

void GameSessionManager::Move(GameSessionRef& _session, uint64 _direction, int64 _movetime)
{
	GameSessionRef gamesession = _session;
	PlayerRef player = gamesession->GetCurrentPlayer();

	unordered_set<uint64> old_vl = gamesession->GetViewPlayer();

	UpdatePlayerPosition(player, _direction);

	unordered_set<uint64> new_vl = ROOMMANAGER->ViewList(gamesession, false); // ������ �� �ֺ��ֵ� + WakeNPC���� ����Ϸ�

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

void GameSessionManager::UpdatePlayerPosition(PlayerRef& _player, uint64 _direction)
{
	PlayerRef player = _player;
	POS pos{ player->POSX, player->POSY };

	switch (_direction)
	{
	case LEFT:
		if (pos.posx > 0 && MAPDATA.GetTile(pos.posy, pos.posx - 1) != MAPDATA.e_OBSTACLE)
			--pos.posx;
		break;
	case RIGHT:
		if (pos.posx < W_WIDTH - 1 && MAPDATA.GetTile(pos.posy, pos.posx + 1) != MAPDATA.e_OBSTACLE)
			++pos.posx;
		break;
	case UP:
		if (pos.posy > 0 && MAPDATA.GetTile(pos.posy - 1, pos.posx) != MAPDATA.e_OBSTACLE)
			--pos.posy;
		break;
	case DOWN:
		if (pos.posy < W_HEIGHT - 1 && MAPDATA.GetTile(pos.posy + 1, pos.posx) != MAPDATA.e_OBSTACLE)
			++pos.posy;
		break;
	}

	player->SetPos(pos);
}

void GameSessionManager::HandleCollisions(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _new_vl)
{
	GameSessionRef gamesession = _gamesession;
	PlayerRef player = _player;

	{
		READ_LOCK;
		for (const auto& vl : _new_vl)
		{
			if (sessions[vl]->GetCurrentPlayer()->GetPT() != Protocol::PLAYER_TYPE_CLIENT && sessions[vl]->GetCurrentPlayer()->GetPT() != Protocol::PLAYER_TYPE_DUMMY)
			{
				if (sessions[vl]->GetCurrentPlayer()->GetPos() == player->GetPos())
				{

					int damage = sessions[vl]->GetCurrentPlayer()->GetStat().level * 5 - player->GetStat().defencePower;
					if (damage <= 0)
						damage = 1;

					player->UpdateStatHp(-damage);
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
						ss << "Damage " << damage << " <- " << sessions[vl]->GetCurrentPlayer()->GetName();
						string chat = ss.str();
						gamesession->ChatPkt(player->GetId(), chat);
					}
				}
			}
		}
	}
}

void GameSessionManager::HandlePlayerDeath(GameSessionRef& _gamesession, PlayerRef& _player, uint64 _killerId)
{
	GameSessionRef gamesession = _gamesession;
	PlayerRef player = _player;

	for (int i = 0; i < MAX_USER; ++i)
	{
		READ_LOCK;
		if (sessions[i] != nullptr)
			sessions[i]->RemovePkt(player->GetId());
	}
	player->SetState(ST_FREE);
	gamesession->ResetViewPlayer();
	ROOMMANAGER->LeaveRoom(gamesession);

	stringstream ss;
	ss << sessions[_killerId]->GetCurrentPlayer()->GetName() << " Die ";
	string chat = ss.str();
	gamesession->ChatPkt(player->GetId(), chat);
	GLogger::Log(spdlog::level::info, player->GetName(), "Death");
	DoTimer(10, &GameSessionManager::PlayerRespawn, player->GetId());
}

void GameSessionManager::UpdateViewList(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl, int64 _movetime)
{
	GameSessionRef gamesession = _gamesession;
	PlayerRef player = _player;

	{
		READ_LOCK;
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
	}

	for (const auto& pl : _old_vl)
	{
		if (_new_vl.count(pl) == 0)
		{
			if (player->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
				gamesession->RemovePkt(pl);
			gamesession->RemoveViewPlayer(pl);

			{
				READ_LOCK;
				if (sessions[pl]->GetViewPlayer().count(player->GetId()))
				{
					if (sessions[pl]->GetCurrentPlayer()->GetPT() == Protocol::PLAYER_TYPE_CLIENT)
						sessions[pl]->RemovePkt(player->GetId());
					sessions[pl]->RemoveViewPlayer(player->GetId());
				}
			}
		}
	}
}

void GameSessionManager::HandleNpcCollision(PlayerRef& _player, uint64 _pl)
{
	GameSessionRef gamesession;
	{
		READ_LOCK;
		gamesession = sessions[_pl];
	}

	PlayerRef player = gamesession->GetCurrentPlayer();

	{
		if (player->POSX == _player->POSX && player->POSY == _player->POSY)
		{
			
			int32 damage = _player->GetStat().level * 5 - player->GetStat().defencePower;
			if (damage <= 0)
				return;
			
			player->UpdateStatHp(-damage);
			

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
			ss << player->GetName() << " <- Damage " << damage << " Attack ";
			string chat = ss.str();
			gamesession->ChatPkt(_pl, chat);

			if (player->GetStat().hp <= 0)
			{
				player->SetState(ST_FREE);
				DoTimer(10, &GameSessionManager::PlayerRespawn, _pl);
			}
		}
	}
}

void GameSessionManager::HandleNPCViewListChanges(GameSessionRef& _gamesession, PlayerRef& _player, unordered_set<uint64>& _old_vl, unordered_set<uint64>& _new_vl)
{
	GameSessionRef gamesession = _gamesession;
	PlayerRef player = _player;

	READ_LOCK;

	for (auto pl : _new_vl)
	{
		if (_old_vl.count(pl) == 0)
		{
			POS pos = { player->POSX, player->POSY };
			{
				if (sessions[pl])
				{
					sessions[pl]->AddObjectPkt(player->GetPT(), player->GetId(), player->GetName(), pos);
					sessions[pl]->AddViewPlayer(player->GetId());
				}
			}
		}
		else
		{
			auto now = std::chrono::system_clock::now();
			std::chrono::seconds since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
			std::int64_t posix_time = since_epoch.count();

			POS pos = { player->POSX, player->POSY };
			{
				if (sessions[pl])
				{
					sessions[pl]->MovePkt(player->GetId(), pos, posix_time);

					if (player->GetId() > MAX_USER)
						HandleNpcCollision(player, pl);

				}
			}
		}
	}

	for (auto pl : _old_vl)
	{
		if (_new_vl.count(pl) == 0)
		{
			gamesession->RemoveViewPlayer(pl);

			{
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
}

void GameSessionManager::MoveNpcToRandomPosition(GameSessionRef& _gamesession, PlayerRef& _player)
{
	GameSessionRef gamesession = _gamesession;
	PlayerRef player = _player;
	
	vector<int> indices = gamesession->GetRandomDirectionIndices();

	for (int index : indices) {
		pair<int, int> pp = directions[index];
		int newX = player->POSX + pp.first;
		int newY = player->POSY + pp.second;

		if (newX >= 0 && newX < W_WIDTH && newY >= 0 && newY < W_HEIGHT && MAPDATA.GetTile(newY, newX) == MAPDATA.e_PLAT) {
			player->SetPos({ newX, newY });
			break;
		}
	}
}

bool GameSessionManager::IsAdjacent(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		READ_LOCK;
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];
	}
	PlayerRef attack = attack_gamesession->GetCurrentPlayer();
	PlayerRef target = target_gamesession->GetCurrentPlayer();

	return (target->POSX == attack->POSX && target->POSY == attack->POSY - 1) ||
		(target->POSX == attack->POSX && target->POSY == attack->POSY + 1) ||
		(target->POSX == attack->POSX - 1 && target->POSY == attack->POSY) ||
		(target->POSX == attack->POSX + 1 && target->POSY == attack->POSY);
}

void GameSessionManager::HandleAttack(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		READ_LOCK;
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];
	}

	PlayerRef monster = target_gamesession->GetCurrentPlayer();
	PlayerRef user = attack_gamesession->GetCurrentPlayer();


	if (monster->GetState() == ST_INGAME)
	{

		int damage = user->GetStat().level * 3 + user->GetStat().attackPower;
		monster->UpdateStatHp(-damage);

		if (monster->GetStat().hp <= 0) // Target died
			HandleNPCDeath(_attackerId, _targetId);
	}
}

void GameSessionManager::HandleNPCDeath(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		READ_LOCK;
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];
	}

	PlayerRef monster = target_gamesession->GetCurrentPlayer();
	PlayerRef user = attack_gamesession->GetCurrentPlayer();
	monster->active = false;
	monster->SetState(ST_FREE);
	user->UpdateStatExp(target_gamesession->GetCurrentPlayer()->GetStat().exp);

	DropItems(target_gamesession, attack_gamesession);

	if (user->GetStat().exp >= user->GetStat().maxExp) // Level up
	{
		user->LevelUp();
		stringstream ss;
		ss << "LV " << (attack_gamesession->GetCurrentPlayer()->GetStat().level - 1) << " -> " << attack_gamesession->GetCurrentPlayer()->GetStat().level << " UP";
		string chat = ss.str();
		attack_gamesession->ChatPkt(_attackerId, chat);
	}
	else
	{
		stringstream ss;
		ss << "Catch " << (target_gamesession->GetCurrentPlayer()->GetName()) << " -> " << target_gamesession->GetCurrentPlayer()->GetStat().exp << " EXP";
		string chat = ss.str();
		attack_gamesession->ChatPkt(_attackerId, chat);
	}

	attack_gamesession->StatChangePkt(
		user->GetStat().level,
		user->GetStat().hp,
		user->GetStat().maxHp,
		user->GetStat().mp,
		user->GetStat().maxMp,
		user->GetStat().exp,
		user->GetStat().maxExp
	);

	{
		READ_LOCK;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (sessions[i] != nullptr)
				sessions[i]->RemovePkt(_targetId);
		}
	}

	target_gamesession->GetCurrentPlayer()->SetState(ST_SLEEP);
	ROOMMANAGER->LeaveRoom(sessions[_targetId]);
	DoTimer(10000, &GameSessionManager::Respawn, target_gamesession);
}

void GameSessionManager::BroadcastAttackMessage(uint64 _attackerId, uint64 _targetId)
{
	GameSessionRef attack_gamesession;
	GameSessionRef target_gamesession;
	{
		READ_LOCK;
		attack_gamesession = sessions[_attackerId];
		target_gamesession = sessions[_targetId];
	}

	stringstream ss;
	ss << attack_gamesession->GetCurrentPlayer()->GetName() << " Attack -> " << target_gamesession->GetCurrentPlayer()->GetName();
	string chat = ss.str();

	attack_gamesession->ChatPkt(_attackerId, chat);

	unordered_set<uint64> viewList = attack_gamesession->GetViewPlayer();

	{
		READ_LOCK;
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
}

void GameSessionManager::DropItems(GameSessionRef& _gamesession, GameSessionRef& _usersession)
{
	GameSessionRef gamesession = _gamesession;
	GameSessionRef usersession = _usersession;
	PlayerRef monster = gamesession->GetCurrentPlayer();
	PlayerRef user = usersession->GetCurrentPlayer();

	int monsterId = static_cast<int>(monster->GetPT()) - 1;
	auto dropTable = GDROPTABLE.GetDropTable(monsterId);
	if (dropTable == nullptr)
		return;

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> chance(0.0, 1.0);

	for (const auto& drop : dropTable->drops)
	{
		float roll = static_cast<float>(chance(gen));
		if (roll <= drop.dropRate)
		{
			uniform_int_distribution<> quantityGen(drop.minQuantity, drop.maxQuantity);
			int quantity = quantityGen(gen);

			if (drop.itemId == GOLD) {
				GLogger::LogWithContext(spdlog::level::info, user->GetName(), 
					"DropItems - Gold","quantity: {}", quantity);
				user->AddGold(quantity); 
				continue;
			}

			auto item = ITEM.GetItem(drop.itemId);
			auto slotIndex = user->GetEmptyIndex(item->itemType);
			if (slotIndex < 0)
				continue; 
			GLogger::LogWithContext(spdlog::level::info, user->GetName(), "DropItems - AddItem", "itemId: {} quantity: {}", 
				drop.itemId, quantity);

			user->AddItem(drop.itemId, item->itemType, slotIndex, quantity);
			stringstream ss;
			ss << ITEM.GetItem(drop.itemId)->name << " acquire!";
			string s = ss.str();
			usersession->ChatPkt(usersession->GetId(), s);
		}
	}
}

void GameSessionManager::UpdateGold(const string& _name, const int _newGold)
{
	vector<RankingData> newTop5;
	if (REDIS.UpdatePlayerGold(_name, _newGold, newTop5))
	{
		READ_LOCK;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (sessions[i] != nullptr)
				sessions[i]->RankingPkt(newTop5);
		}
	}
}

void GameSessionManager::UpdateGold()
{
	auto top5 = REDIS.GetTop5Ranking();
	READ_LOCK;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (sessions[i] != nullptr)
			sessions[i]->RankingPkt(top5);
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
		WRITE_LOCK;
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

		if (MAPDATA.GetTile(y, x) == MAPDATA.e_PLAT) {
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

