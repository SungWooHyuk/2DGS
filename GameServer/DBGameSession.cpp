#include "pch.h"
#include "DBGameSession.h"
#include "DBGameSessionManager.h"
#include "GLogger.h"
#include "GameDBPacketHandler.h"

void DBGameSession::OnConnected()
{
	DBMANAGER.SetSession(static_pointer_cast<DBGameSession>(shared_from_this()));
	GLogger::Log(spdlog::level::info, "DBServer Connect");
}

void DBGameSession::OnDisconnected()
{
	DBMANAGER.RemoveSession();
	GLogger::Log(spdlog::level::info, "DBServer DisConnect");
}

void DBGameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	GameDBPacketHandler::HandlePacket(session, buffer, len);
}

void DBGameSession::OnSend(int32 len)
{
}

void DBGameSession::SendInventoryPkt(uint64 _id, const string& _name, const InventoryMap& _inventory)
{
	GLogger::LogWithContext(spdlog::level::info, _name, "SendInventoryPkt", "Sending {} items to DB", _inventory.size());

	DBProtocol::SD_SAVE_INVENTORY pkt;
	pkt.set_user_id(_id);
	pkt.set_name(_name);

	for (const auto& [key, item] : _inventory) {
		auto* slot = pkt.add_inventory();
		slot->set_item_id(item.itemId);
		slot->set_quantity(item.quantity);
		slot->set_inv_slot_index(item.slot_index);
		slot->set_tab_type(item.tab_type);
	}

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void DBGameSession::SendEquipmentPkt(uint64 _id, const string& _name, const EquipmentMap& _equipment) {
	GLogger::LogWithContext(spdlog::level::info, _name, "SendEquipmentPkt", "Sending {} equipment slots to DB", _equipment.size());

	DBProtocol::SD_SAVE_EQUIPMENT pkt;
	pkt.set_user_id(_id);
	pkt.set_name(_name);

	for (const auto& [slot, item] : _equipment) {
		auto* equip = pkt.add_equipment();
		equip->set_eq_slot(slot);
		equip->set_item_id(item.itemId);
	}

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void DBGameSession::SendPlayerStatePkt(uint64 _id, const string& _name, const POS& _pos, const STAT& _stats) {
	
	DBProtocol::SD_SAVE_PLAYER pkt;
	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void DBGameSession::SendUpdateGoldPkt(uint64 _id, const string& _name, int _gold)
{
	GLogger::LogWithContext(spdlog::level::info, _name, "SendUpdateGoldPkt", "Sending updated gold: {}", _gold);

	DBProtocol::SD_UPDATE_GOLD pkt;
	pkt.set_gold(_gold);
	pkt.set_name(_name);
	pkt.set_user_id(_id);
	
	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void DBGameSession::SendSavePkt(uint64 _id, const InventoryMap& _inven, const EquipmentMap& _equip, const USER_INFO& _player)
{
	GLogger::LogWithContext(spdlog::level::info, _player.name, "SendSavePkt",
		"Saving to DB: Pos=({}, {}), Level={}, HP={}/{} MP={}/{} Gold={}, Equip={}, Inven={}",
		_player.posx, _player.posy, _player.level, _player.hp, _player.maxHp, _player.mp, _player.maxMp, _player.gold,
		_equip.size(), _inven.size());

	DBProtocol::SD_SAVE_PLAYER pkt;
	pkt.set_user_id(_id);
	pkt.set_name(_player.name);

	for (const auto& [slot, item] : _equip) {
		auto* equip = pkt.add_equipment();
		equip->set_eq_slot(slot);
		equip->set_item_id(item.itemId);
	}

	for (const auto& [key, item] : _inven) {
		auto* slot = pkt.add_inventory();
		slot->set_item_id(item.itemId);
		slot->set_quantity(item.quantity);
		slot->set_inv_slot_index(item.slot_index);
		slot->set_tab_type(item.tab_type);
	}

	auto* player = pkt.add_player();
	player->add_playertype(Protocol::PlayerType::PLAYER_TYPE_CLIENT);
	player->set_id(_id);
	player->set_name(_player.name);
	player->set_x(_player.posx);
	player->set_y(_player.posy);
	player->set_level(_player.level);
	player->set_hp(_player.hp);
	player->set_maxhp(_player.maxHp);
	player->set_mp(_player.mp);
	player->set_maxmp(_player.maxMp);
	player->set_exp(_player.exp);
	player->set_maxexp(_player.maxExp);
	player->set_gold(_player.gold);

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}
