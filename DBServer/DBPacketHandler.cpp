#include "pch.h"
#include "DBPacketHandler.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}
bool Handle_SD_LOGIN(PacketSessionRef& session, DBProtocol::SD_LOGIN& pkt)
{
	return false;
}

bool Handle_SD_SAVE_PLAYER(PacketSessionRef& session, DBProtocol::SD_SAVE_PLAYER& pkt)
{
	return false;
}

bool Handle_SD_GET_INVENTORY(PacketSessionRef& session, DBProtocol::SD_GET_INVENTORY& pkt)
{
	return false;
}

bool Handle_SD_UPDATE_INVENTORY(PacketSessionRef& session, DBProtocol::SD_UPDATE_INVENTORY& pkt)
{
	return false;
}
