#include "pch.h"
#include "DBSession.h"
#include "DBPacketHandler.h"
#include "GLogger.h"

PacketHandlerFunc GDBPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}
bool Handle_SD_LOGIN(PacketSessionRef& session, DBProtocol::SD_LOGIN& pkt)
{
	DBSessionRef dbsession = static_pointer_cast<DBSession>(session);
	string pktName = pkt.name();
	uint64 pktId = pkt.user_id();

	if (!dbsession->IsUserExists(pktName))
		dbsession->RegisterNewUser(pktName);
	
	if(dbsession->GetUserInfo(pktName, pktId))
		return true;

	return false;
}

bool Handle_SD_SAVE_PLAYER(PacketSessionRef& session, DBProtocol::SD_SAVE_PLAYER& pkt)
{
	DBSessionRef dbsession = static_pointer_cast<DBSession>(session);
	string pktName = pkt.name();
	uint64 pktId = pkt.user_id();
	stringstream inventoryLogs;
	stringstream equipmentLog;

	USER_INFO user;
	const auto& p = pkt.player(0);

	user.exp = p.exp();
	user.gold = p.gold();
	user.hp = p.hp();
	user.level = p.level();
	user.maxExp = p.maxexp();
	user.maxHp = p.maxhp();
	user.maxMp = p.maxmp();
	user.mp = p.mp();
	user.posx = p.x();
	user.posy = p.y();
	user.name = p.name();

	dbsession->UpdateUserInfo(user); // 현재 정보
	

	for (const auto& slot : pkt.inventory()) // 인벤
	{
		if (!dbsession->InsertInventorySlot(pktName, slot))
			return false;

		inventoryLogs << fmt::format("[tab:{} item:{} qty:{} slot:{}] ",
			static_cast<int>(slot.tab_type()), slot.item_id(), slot.quantity(), slot.inv_slot_index());
	}
	dbsession->UpdateUserEquipment(pkt.name(), { pkt.equipment().begin(), pkt.equipment().end() }); // 장비

	for (const auto& item : pkt.equipment()) 
		equipmentLog << fmt::format("[slot:{} item:{}] ", static_cast<int>(item.eq_slot()), item.item_id());
	

	GLogger::Log(spdlog::level::info,
		"SAVE_PLAYER user: {} POS: [ {},{} ] level: {} hp: [{}/{}] mp: [{}/{}] exp: [{}/{}] gold: {}\n - Inventory: {}\n - Equipment: {}",
		user.name, user.posx, user.posy, user.level, user.hp, user.maxHp, user.mp, user.maxMp, user.exp, user.maxExp, user.gold,
		inventoryLogs.str(), equipmentLog.str());
	return false;
}

bool Handle_SD_GET_INFOMATION(PacketSessionRef& session, DBProtocol::SD_GET_INFOMATION& pkt)
{
	return false;
}

bool Handle_SD_EQUIP_ITEM(PacketSessionRef& session, DBProtocol::SD_EQUIP_ITEM& pkt)
{
	return false;
}

bool Handle_SD_FARMING_ITEM(PacketSessionRef& session, DBProtocol::SD_FARMING_ITEM& pkt)
{
	return false;
}

bool Handle_SD_UNEQUIP_ITEM(PacketSessionRef& session, DBProtocol::SD_UNEQUIP_ITEM& pkt)
{
	return false;
}

bool Handle_SD_CONSUME_ITEM(PacketSessionRef& session, DBProtocol::SD_CONSUME_ITEM& pkt)
{
	return false;
}

bool Handle_SD_MOVE_ITEM(PacketSessionRef& session, DBProtocol::SD_MOVE_ITEM& pkt)
{
	return false;
}

bool Handle_SD_REGISTER(PacketSessionRef& session, DBProtocol::SD_REGISTER& pkt)
{
	return false;
}

bool Handle_SD_SAVE_INVENTORY(PacketSessionRef& session, DBProtocol::SD_SAVE_INVENTORY& pkt)
{
	DBSessionRef dbsession = static_pointer_cast<DBSession>(session);
	dbsession->ClearInventory(pkt.name()); 
	for (const auto& slot : pkt.inventory())
	{
		if (!dbsession->InsertInventorySlot(pkt.name(), slot))
			return false;
	}
	return false;
}

bool Handle_SD_SAVE_EQUIPMENT(PacketSessionRef& session, DBProtocol::SD_SAVE_EQUIPMENT& pkt)
{
	DBSessionRef dbsession = static_pointer_cast<DBSession>(session);
	dbsession->UpdateUserEquipment(pkt.name(), { pkt.equipment().begin(), pkt.equipment().end() });
	return false;
}

bool Handle_SD_UPDATE_GOLD(PacketSessionRef& session, DBProtocol::SD_UPDATE_GOLD& pkt)
{
	GLogger::Log(spdlog::level::info, "UPDATE_GOLD_PKT name: {}, gold: {}", pkt.name(), pkt.gold());
	DBSessionRef dbsession = static_pointer_cast<DBSession>(session);
	dbsession->UpdateUserGold(pkt.name(), pkt.gold());
	return false;
}
