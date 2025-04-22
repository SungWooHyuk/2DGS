#include "pch.h"
#include "utils.h"
#include "GameDBPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Room.h"
#include "Player.h"
#include "RoomManager.h"

bool Handle_DS_LOGIN(PacketSessionRef& session, DBProtocol::DS_LOGIN& pkt)
{
	return true;
}
bool Handle_DS_REGISTER(PacketSessionRef& session, DBProtocol::DS_REGISTER& pkt)
{
	return false;
}

bool Handle_DS_SAVE_RESULT(PacketSessionRef& session, DBProtocol::DS_SAVE_RESULT& pkt)
{
	return false;
}

bool Handle_DS_USER_INFORMATION(PacketSessionRef& session, DBProtocol::DS_USER_INFORMATION& pkt)
{
	return false;
}

bool Handle_DS_INVENTORY_INFORMATION(PacketSessionRef& session, DBProtocol::DS_INVENTORY_INFORMATION& pkt)
{
	return false;
}

bool Handle_DS_EQUIP_INFORMATION(PacketSessionRef& session, DBProtocol::DS_EQUIP_INFORMATION& pkt)
{
	return false;
}

bool Handle_DS_EQUIP_ITEM(PacketSessionRef& session, DBProtocol::DS_EQUIP_ITEM& pkt)
{
	return false;
}

bool Handle_DS_FARMING_RESULT(PacketSessionRef& session, DBProtocol::DS_FARMING_RESULT& pkt)
{
	return false;
}

bool Handle_DS_CONSUME_ITEM(PacketSessionRef& session, DBProtocol::DS_CONSUME_ITEM& pkt)
{
	return false;
}

bool Handle_DS_MOVE_RESULT(PacketSessionRef& session, DBProtocol::DS_MOVE_RESULT& pkt)
{
	return false;
}

bool Handle_DS_UPDATE_GOLD(PacketSessionRef& session, DBProtocol::DS_UPDATE_GOLD& pkt)
{
	return true;
}