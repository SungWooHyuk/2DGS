#include "pch.h"
#include "DBGameSession.h"
#include "DBGameSessionManager.h"
#include "GLogger.h"
#include "GameDBPacketHandler.h"
#include "RedisManager.h"

void DBGameSession::OnConnected()
{
	auto session = static_pointer_cast<DBGameSession>(shared_from_this());
	if (DBMANAGER.GetSession() != session)
		DBMANAGER.SetSession(session);

	REDIS.FlushQueue(session);
}

void DBGameSession::OnDisconnected()
{
	DBMANAGER.RemoveSession();
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

// 이제 이 함수들은 단순히 패킷만 전송 (연결 상태 체크 없음)
void DBGameSession::SendInventoryPkt(uint64 _id, const string& _name, const InventoryMap& _inventory)
{
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

void DBGameSession::SendEquipmentPkt(uint64 _id, const string& _name, const EquipmentMap& _equipment)
{
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

void DBGameSession::SendPlayerStatePkt(uint64 _id, const string& _name, const POS& _pos, const STAT& _stats)
{
	DBProtocol::SD_SAVE_PLAYER pkt;
	// TODO: 필요한 필드들 설정

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void DBGameSession::SendUpdateGoldPkt(uint64 _id, const string& _name, int _gold)
{
	DBProtocol::SD_UPDATE_GOLD pkt;
	pkt.set_gold(_gold);
	pkt.set_name(_name);
	pkt.set_user_id(_id);

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void DBGameSession::SendSavePkt(uint64 _id, const InventoryMap& _inven, const EquipmentMap& _equip, const USER_INFO& _player)
{
	DBProtocol::SD_SAVE_PLAYER pkt;
	pkt.set_user_id(_id);
	pkt.set_name(_player.name);

	for (const auto& [slot, item] : _equip) {
		auto* equipment = pkt.add_equipment();
		equipment->set_eq_slot(slot);
		equipment->set_item_id(item.itemId);
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