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

bool Handle_DS_UPDATE_GOLD(PacketSessionRef& session, DBProtocol::DS_UPDATE_GOLD& pkt)
{
	return true;
}