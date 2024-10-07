#include "pch.h"
#include "utils.h"
#include "MapData.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "RoomManager.h"

void GameSession::OnConnected()
{
	GAMESESSIONMANAGER->Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GAMESESSIONMANAGER->Remove(static_pointer_cast<GameSession>(shared_from_this()));

	if (currentPlayer)
	{
		if (auto rooms = room.lock())
			rooms->DoAsync(&Room::Leave, currentPlayer);
	}
	WRITE_LOCK;
	currentPlayer = nullptr;
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO : packetId 대역 체크
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}

bool GameSession::IsNpc(uint64 _myId)
{
	return _myId > MAX_USER;
}

bool GameSession::DoNpcRandomMove(GameSessionRef& _session)
{
	return false;
}

vector<int> GameSession::GetRandomDirectionIndices()
{
	std::vector<int> indices = { 0, 1, 2, 3 };
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(indices.begin(), indices.end(), g);
	return indices;
}

bool GameSession::CanGo(POS _pos)
{
	if (MAPDATA->GetTile(_pos.posy, _pos.posx) == MAPDATA->e_PLAT)
		return true;
	return false;
}

void GameSession::ResetPath()
{
	if (!path.empty()) {
		path.clear();
		pathIndex = 1;
		pathCount = 0;
	}
}

void GameSession::SetPath(POS _dest, map<POS, POS>& _parent)
{
	POS pos = _dest;

	path.clear();
	pathIndex = 1;
	pathCount = 0;
	while (true)
	{
		path.push_back(pos);

		if (pos == _parent[pos])
			break;

		pos = _parent[pos];
	}

	reverse(path.begin(), path.end());
}

bool GameSession::EmptyPath()
{
	if (path.empty())
		return true;
	else
		return false;
}

void GameSession::RemovePkt(uint64 _id)
{
	Protocol::S_REMOVE_OBJECT removePkt;
	removePkt.set_id(_id);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(removePkt);
	Send(sendBuffer);
}

void GameSession::RespawnPkt(int32 _hp, POS _pos, int32 _exp)
{
	Protocol::S_RESPAWN respawnPkt;
	respawnPkt.set_hp(_hp);
	respawnPkt.set_x(_pos.posx);
	respawnPkt.set_x(_pos.posy);
	respawnPkt.set_exp(_exp);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(respawnPkt);
	Send(sendBuffer);
}

void GameSession::AddObjectPkt(Protocol::PlayerType _pt, uint64 _id, string _name, POS _pos)
{
	Protocol::S_ADD_OBJECT addPkt;
	auto player = addPkt.add_players();
	player->add_playertype(_pt);
	player->set_id(_id);
	player->set_name(_name);
	player->set_x(_pos.posx);
	player->set_y(_pos.posy);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(addPkt);
	Send(sendBuffer);
}

void GameSession::MovePkt(uint64 _id, POS _pos, int64 _time)
{
	Protocol::S_MOVE_OBJECT movePkt;
	auto move = movePkt.add_moves();
	move->set_id(_id);
	move->set_x(_pos.posx);
	move->set_y(_pos.posy);
	move->set_move_time(_time);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
	Send(sendBuffer);
}

void GameSession::DamagePkt(int32 _damage)
{
	Protocol::S_DAMAGE damagePkt;
	damagePkt.set_damage(_damage);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(damagePkt);
	Send(sendBuffer);
}

void GameSession::ChatPkt(uint64 _id, string _mess)
{
	Protocol::S_CHAT chatPkt;
	auto chat = chatPkt.add_messages();
	chat->set_id(_id);
	chat->set_mess(_mess);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);
	Send(sendBuffer);
}

void GameSession::StatChangePkt(int32 _level, int32 _hp, int32 _maxhp, int32 _mp, int32 _maxmp, int32 _exp, int32 _maxexp)
{
	Protocol::S_STAT_CHANGE statPkt;
	auto stat = statPkt.add_stats();
	stat->set_level(_level);
	stat->set_hp(_hp);
	stat->set_maxhp(_maxhp);
	stat->set_mp(_mp);
	stat->set_maxmp(_maxmp);
	stat->set_exp(_exp);
	stat->set_maxexp(_maxexp);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(statPkt);
	Send(sendBuffer);
}

void GameSession::LoginPkt(bool _success, uint64 _id, Protocol::PlayerType _pt, string _name, POS _pos, STAT _stat)
{
	Protocol::S_LOGIN sPkt;
	sPkt.set_success(_success);
	auto Player = sPkt.add_players();
	Player->add_playertype(_pt);
	Player->set_id(_id);
	Player->set_name(_name);
	Player->set_x(_pos.posx);
	Player->set_y(_pos.posy);
	Player->set_exp(_stat.exp);
	Player->set_hp(_stat.hp);
	Player->set_mp(_stat.mp);
	Player->set_level(_stat.level);
	Player->set_maxexp(_stat.maxExp);
	Player->set_maxhp(_stat.maxHp);
	Player->set_maxmp(_stat.maxMp);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(sPkt);
	Send(sendBuffer);
}

void GameSession::LoginPkt(bool _success)
{
	Protocol::S_LOGIN sPkt;
	sPkt.set_success(false);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(sPkt);
	Send(sendBuffer);
}

