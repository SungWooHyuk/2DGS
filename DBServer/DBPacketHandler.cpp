#include "pch.h"
#include "DBSession.h"
#include "DBPacketHandler.h"

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
	return false;
}

bool Handle_SD_SAVE_EQUIPMENT(PacketSessionRef& session, DBProtocol::SD_SAVE_EQUIPMENT& pkt)
{
	return false;
}

bool Handle_SD_UPDATE_GOLD(PacketSessionRef& session, DBProtocol::SD_UPDATE_GOLD& pkt)
{
	return false;
}
