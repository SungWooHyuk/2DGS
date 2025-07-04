#include "pch.h"
#include "utils.h"
#include "MapData.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "GameDBPacketHandler.h"
#include "Player.h"
#include "RoomManager.h"
#include "StateSnap.h"
#include "GLogger.h"
#include "JobQueue.h"

GameSession::GameSession()
{
	jobQueue = MakeShared<JobQueue>();
}

void GameSession::OnConnected()
{
	GAMESESSIONMANAGER->Add(static_pointer_cast<GameSession>(shared_from_this()));
	//GLogger::LogWithContext(spdlog::level::info, "GameSession", "OnConnected", "New");
}

void GameSession::OnDisconnected()
{
	if (currentPlayer) {
		GLogger::LogWithContext(spdlog::level::info, currentPlayer->GetName(), "OnDisconnected",
			"Player disconnected. ID: {}", currentPlayer->GetId());
	}
	GAMESESSIONMANAGER->Remove(static_pointer_cast<GameSession>(shared_from_this()));

	WRITE_LOCK_IDX(0);
	PlayerRef player = currentPlayer;
	currentPlayer = nullptr;

	if (auto rooms = room.lock())
		rooms->DoAsync(&Room::Leave, player);

}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	ClientPacketHandler::HandlePacket(session, buffer, len);

}

void GameSession::OnSend(int32 len)
{
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
	if (_pos.posx < 0 || _pos.posx >= W_WIDTH || _pos.posy < 0 || _pos.posy >= W_HEIGHT)
		return false;

	if (MAPDATA.GetTile(_pos.posy, _pos.posx) == MAPDATA.e_OBSTACLE)
		return false;

	return true;
}

void GameSession::ResetPath()
{
	WRITE_LOCK_IDX(2);
	if (!path.empty()) {
		path.clear();
		pathIndex = 1;
		pathCount = 0;
	}
}

void GameSession::SetPath(POS _dest, map<POS, POS>& _parent)
{
	WRITE_LOCK_IDX(2);
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
	READ_LOCK_IDX(2);
	if (path.empty())
		return true;
	else
		return false;
}

bool GameSession::SaveStateSnap()
{
	READ_LOCK_IDX(0);

	if (currentPlayer == nullptr) {
		GLogger::Log(spdlog::level::err, "SaveStateSnap , currentPlayer id={} nullptr err", myId);
		return false;
	}
	currentPlayer->GetStateSnap().SaveState();
	return true;
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

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(sPkt);
	Send(sendBuffer);
}

void GameSession::LoginPkt(bool _success, PlayerRef _player)
{
	Protocol::S_LOGIN pkt;
	pkt.set_success(_success);

	auto Player = pkt.add_players();
	Player->add_playertype(_player->GetPT());
	Player->set_id(_player->GetId());
	Player->set_name(_player->GetName());
	Player->set_x(_player->GetPos().posx);
	Player->set_y(_player->GetPos().posy);
	Player->set_gold(_player->GetGold());

	_player->UpdateStatByEquipment();

	const STAT& stats = _player->GetStat();
	auto Stat = pkt.add_stats();

	Stat->set_exp(stats.exp);
	Stat->set_hp(stats.hp);
	Stat->set_mp(stats.mp);
	Stat->set_level(stats.level);
	Stat->set_maxexp(stats.maxExp);
	Stat->set_maxhp(stats.maxHp);
	Stat->set_maxmp(stats.maxMp);
	Stat->set_attackpower(stats.attackPower);
	Stat->set_defensepower(stats.defencePower);
	Stat->set_magicpower(stats.magicPower);
	Stat->set_strenth(stats.strength);

	auto inven = pkt.mutable_inventory();
	for (const auto& [slot, item] : _player->GetInventory())
	{
		auto slotData = inven->add_inventory();
		slotData->set_item_id(item.itemId);
		slotData->set_quantity(item.quantity);
		slotData->set_tab_type(item.tab_type);
		slotData->set_inv_slot_index(item.slot_index);
	}

	for (const auto& [slot, itemInfo] : _player->GetEquipments())
	{
		if (itemInfo.itemId == 0)
			continue;

		auto equip = pkt.mutable_equipment()->add_equipment();
		switch (slot)
		{
		case Protocol::WEAPON:
			equip->set_eq_slot(Protocol::WEAPON);
			equip->set_item_id(itemInfo.itemId);
			break;
		case Protocol::HELMET:
			equip->set_eq_slot(Protocol::HELMET);
			equip->set_item_id(itemInfo.itemId);
			break;
		case Protocol::TOP:
			equip->set_eq_slot(Protocol::TOP);
			equip->set_item_id(itemInfo.itemId);
			break;
		case Protocol::BOTTOM:
			equip->set_eq_slot(Protocol::BOTTOM);
			equip->set_item_id(itemInfo.itemId);
			break;
		}
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::LoginPkt(bool _success)
{
	Protocol::S_LOGIN sPkt;
	sPkt.set_success(false);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(sPkt);
	Send(sendBuffer);
}

void GameSession::LoadInventoryPkt()
{
	Protocol::S_LOAD_INVENTORY pkt;

	auto inv = currentPlayer->GetInventory();

	for (const auto& [key, inven] : inv)
	{
		Protocol::InventorySlot* slot = pkt.add_inventory();
		slot->set_item_id(inven.itemId);
		slot->set_inv_slot_index(inven.slot_index);
		slot->set_tab_type(inven.tab_type);
		slot->set_quantity(inven.quantity);
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::LoadEquipmentPkt()
{
	Protocol::S_LOAD_EQUIPMENT pkt;

	auto eup = currentPlayer->GetEquipments();

	for (const auto& [slot, item] : eup)
	{
		Protocol::EquipmentItem* eq = pkt.add_equipment();
		eq->set_eq_slot(slot);
		eq->set_item_id(item.itemId);
	}
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::ConsumeItemPkt(bool _success, uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _newQuantity)
{
	Protocol::S_CONSUME_RESULT pkt;

	pkt.set_success(_success);
	pkt.set_item_id(_itemId);
	pkt.set_new_quantity(_newQuantity);
	pkt.set_tab_type(_tab);
	pkt.set_slot_index(_slotIndex);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::DropPkt(bool _success, uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _dropQuantity)
{
	Protocol::S_DROP_RESULT pkt;
	pkt.set_success(_success);
	pkt.set_item_id(_itemId);
	pkt.set_tab_type(_tab);
	pkt.set_inv_slot_index(_slotIndex);
	pkt.set_quantity_dropped(_dropQuantity);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::MoveInventoryIndex(bool _success, Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex, uint64 _moveItemId, uint64 _quantity)
{
	Protocol::S_MOVE_INVENTORY_RESULT pkt;

	pkt.set_success(_success);
	pkt.set_from_tab(_fromTab);
	pkt.set_inv_from_index(_fromIndex);
	pkt.set_to_tab(_toTab);
	pkt.set_inv_to_index(_toIndex);
	pkt.set_moved_item_id(_moveItemId);
	pkt.set_quantity(_quantity);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::EquipPkt(bool _success, uint64 _itemId, Protocol::EquipmentSlot _slot)
{
	Protocol::S_EQUIP_RESULT pkt;
	pkt.set_success(_success);
	pkt.set_item_id(_itemId);
	pkt.set_slot_type(_slot);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::UnEquipPkt(bool _success, uint64 _itemId, Protocol::EquipmentSlot _slot, Protocol::InventoryTab _tab, uint64 _invToSlotIndex)
{
	Protocol::S_UNEQUIP_RESULT pkt;
	pkt.set_success(_success);
	pkt.set_item_id(_itemId);
	pkt.set_slot_type(_slot);
	pkt.set_to_tab_type(_tab);
	pkt.set_inv_to_slot_index(_invToSlotIndex);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::GoldChangePkt(uint64 _newGold, int64 _delta)
{
	Protocol::S_GOLD_CHANGE pkt;
	pkt.set_delta(_delta);
	pkt.set_new_gold(_newGold);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::RankingPkt(const vector<RankingData>& _ranking)
{
	Protocol::S_RANKING pkt;

	for (const auto& r : _ranking)
	{
		Protocol::GoldRanking* gr = pkt.add_ranking();
		gr->set_name(r.playerName);
		gr->set_gold(r.gold);
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::RemoveItemPkt(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex)
{
	Protocol::S_REMOVE_ITEM pkt;
	pkt.set_item_id(_itemId);
	pkt.set_inv_slot_index(_slotIndex);
	pkt.set_tab(_tab);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::SwapPkt(bool _success, Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex)
{
	Protocol::S_SWAP_ITEM pkt;
	pkt.set_success(_success);
	pkt.set_from_tab(_fromTab);
	pkt.set_inv_from_index(_fromIndex);
	pkt.set_to_tab(_toTab);
	pkt.set_inv_to_index(_toIndex);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void GameSession::StartSaveTimer()
{
	auto self = static_pointer_cast<GameSession>(shared_from_this());
	jobQueue->DoTimer(10000, [self]() {
		self->SaveStateSnap();
		self->StartSaveTimer();
		});
}
