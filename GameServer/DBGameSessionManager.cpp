#include "pch.h"
#include "DBGameSessionManager.h"
#include "GameDBPacketHandler.h"
#include "RedisManager.h"
#include "GLogger.h"

void DBSessionManager::SendInventoryPkt(uint64 id, const string& name, const InventoryMap& inventory)
{
	DBProtocol::SD_SAVE_INVENTORY pkt;
	pkt.set_user_id(id);
	pkt.set_name(name);

	for (const auto& [key, item] : inventory) {
		auto* slot = pkt.add_inventory();
		slot->set_item_id(item.itemId);
		slot->set_quantity(item.quantity);
		slot->set_inv_slot_index(item.slot_index);
		slot->set_tab_type(item.tab_type);
	}

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);

	// 세션 존재 및 연결 상태 확인
	if (dbSession && dbSession->IsConnected()) {
		dbSession->Send(sendBuffer);
	}
	else {
		GLogger::Log(spdlog::level::warn, "DB session not available, enqueueing SendInventoryPkt to Redis. id={}", id);
		REDIS.EnqueuePacket(sendBuffer);
	}
}

void DBSessionManager::SendEquipmentPkt(uint64 id, const string& name, const EquipmentMap& equipment)
{
	DBProtocol::SD_SAVE_EQUIPMENT pkt;
	pkt.set_user_id(id);
	pkt.set_name(name);

	for (const auto& [slot, item] : equipment) {
		auto* equip = pkt.add_equipment();
		equip->set_eq_slot(slot);
		equip->set_item_id(item.itemId);
	}

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);

	if (dbSession && dbSession->IsConnected()) {
		dbSession->Send(sendBuffer);
	}
	else {
		GLogger::Log(spdlog::level::warn, "DB session not available, enqueueing SendEquipmentPkt to Redis. id={}", id);
		REDIS.EnqueuePacket(sendBuffer);
	}
}

void DBSessionManager::SendPlayerStatePkt(uint64 id, const string& name, const POS& pos, const STAT& stats)
{
	DBProtocol::SD_SAVE_PLAYER pkt;
	// TODO: 필요한 필드들 설정

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);

	if (dbSession && dbSession->IsConnected()) {
		dbSession->Send(sendBuffer);
	}
	else {
		GLogger::Log(spdlog::level::warn, "DB session not available, enqueueing SendPlayerStatePkt to Redis. id={}", id);
		REDIS.EnqueuePacket(sendBuffer);
	}
}

void DBSessionManager::SendUpdateGoldPkt(uint64 id, const string& name, int gold)
{
	DBProtocol::SD_UPDATE_GOLD pkt;
	pkt.set_gold(gold);
	pkt.set_name(name);
	pkt.set_user_id(id);

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);

	if (dbSession && dbSession->IsConnected()) {
		dbSession->Send(sendBuffer);
	}
	else {
		GLogger::Log(spdlog::level::warn, "DB session not available, enqueueing SendUpdateGoldPkt to Redis. id={}", id);
		REDIS.EnqueuePacket(sendBuffer);
	}
}

void DBSessionManager::SendSavePkt(uint64 id, const InventoryMap& inven, const EquipmentMap& equip, const USER_INFO& player)
{
	DBProtocol::SD_SAVE_PLAYER pkt;
	pkt.set_user_id(id);
	pkt.set_name(player.name);

	for (const auto& [slot, item] : equip) {
		auto* equipment = pkt.add_equipment();
		equipment->set_eq_slot(slot);
		equipment->set_item_id(item.itemId);
	}

	for (const auto& [key, item] : inven) {
		auto* slot = pkt.add_inventory();
		slot->set_item_id(item.itemId);
		slot->set_quantity(item.quantity);
		slot->set_inv_slot_index(item.slot_index);
		slot->set_tab_type(item.tab_type);
	}

	auto* playerInfo = pkt.add_player();
	playerInfo->add_playertype(Protocol::PlayerType::PLAYER_TYPE_CLIENT);
	playerInfo->set_id(id);
	playerInfo->set_name(player.name);
	playerInfo->set_x(player.posx);
	playerInfo->set_y(player.posy);
	playerInfo->set_level(player.level);
	playerInfo->set_hp(player.hp);
	playerInfo->set_maxhp(player.maxHp);
	playerInfo->set_mp(player.mp);
	playerInfo->set_maxmp(player.maxMp);
	playerInfo->set_exp(player.exp);
	playerInfo->set_maxexp(player.maxExp);
	playerInfo->set_gold(player.gold);

	auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);

	if (dbSession && dbSession->IsConnected()) {
		dbSession->Send(sendBuffer);
	}
	else {
		GLogger::Log(spdlog::level::warn, "DB session not available, enqueueing SendSavePkt to Redis. id={}", id);
		REDIS.EnqueuePacket(sendBuffer);
	}
}