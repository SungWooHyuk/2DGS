#include "pch.h"
#include "utils.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Room.h"
#include "Player.h"
#include "RoomManager.h"
#include "GameDBPacketHandler.h"
#include "DBGameSessionManager.h"
#include "DBGameSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	string pktName = pkt.name();
	
	if (pktName.substr(0, 5) == "dummy")
	{
		// 더미 로그인 따로 처리
	}
	else
	{
		DBProtocol::SD_LOGIN pkt;
		pkt.set_name(pktName);
		pkt.set_user_id(gamesession->GetId());
		auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
		DBMANAGER.GetSession()->Send(sendBuffer);
	}

	return true;
}

bool Handle_C_CONSUME_ITEM(PacketSessionRef& session, Protocol::C_CONSUME_ITEM& pkt)
{
	return false;
}

bool Handle_C_DROP_ITEM(PacketSessionRef& session, Protocol::C_DROP_ITEM& pkt)
{
	return false;
}

bool Handle_C_MOVE_INVENTORY_ITEM(PacketSessionRef& session, Protocol::C_MOVE_INVENTORY_ITEM& pkt)
{
	return false;
}

bool Handle_C_EQUIP(PacketSessionRef& session, Protocol::C_EQUIP& pkt)
{
	return false;
}

bool Handle_C_UNEQUIP(PacketSessionRef& session, Protocol::C_UNEQUIP& pkt)
{
	return false;
}

bool Handle_C_SORT_INVENTORY(PacketSessionRef& session, Protocol::C_SORT_INVENTORY& pkt)
{
	return false;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	uint64 direction = pkt.direction();
	int64 move_time = pkt.move_time();
	GAMESESSIONMANAGER.Move(gamesession, direction, move_time);

	return false;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	string mess = pkt.msg();
	GAMESESSIONMANAGER.Chat(gamesession, mess);

	return false;
}

bool Handle_C_TELEPORT(PacketSessionRef& session, Protocol::C_TELEPORT& pkt)
{
	// Stress test ��
	return false;
}

bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	GAMESESSIONMANAGER.Attack(gamesession, pkt.id(), pkt.skill());

	return false;
}

bool Handle_C_LOGOUT(PacketSessionRef& session, Protocol::C_LOGOUT& pkt)
{
	return false;
}
