#include "pch.h"
#include "TestPacketHandler.h"
#include "DummySession.h"
#include "DummyManager.h"
#include "Client.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	DummySessionRef dummySession = static_pointer_cast<DummySession>(session);
	if (pkt.success())
	{
		for (const auto& cli : pkt.players())
		{
			DUMMYMANAGER.Login(dummySession, cli.id(), cli.x(), cli.y());
			DUMMYMANAGER.Add(dummySession);
			return true;
		}
	}
	return false;
}

bool Handle_S_ADD_OBJECT(PacketSessionRef& session, Protocol::S_ADD_OBJECT& pkt)
{
	return true;
}

bool Handle_S_REMOVE_OBJECT(PacketSessionRef& session, Protocol::S_REMOVE_OBJECT& pkt)
{
	return true;
}

bool Handle_S_MOVE_OBJECT(PacketSessionRef& session, Protocol::S_MOVE_OBJECT& pkt)
{
	DummySessionRef serverSession = static_pointer_cast<DummySession>(session);

	for (const auto& cli : pkt.moves())
	{
		uint64 id = cli.id();
		uint64 x = cli.x();
		uint64 y = cli.y();
		int64  move_time = cli.move_time();

		DUMMYMANAGER.Move(serverSession, id, x, y, move_time);
	}

	return true;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	return true;
}

bool Handle_S_STAT_CHANGE(PacketSessionRef& session, Protocol::S_STAT_CHANGE& pkt)
{
	return true;
}

bool Handle_S_DAMAGE(PacketSessionRef& session, Protocol::S_DAMAGE& pkt)
{
	return true;
}

bool Handle_S_RESPAWN(PacketSessionRef& session, Protocol::S_RESPAWN& pkt)
{
	return true;
}

bool Handle_S_LOAD_INVENTORY(PacketSessionRef& session, Protocol::S_LOAD_INVENTORY& pkt)
{
	return true;
}
bool Handle_S_LOAD_EQUIPMENT(PacketSessionRef& session, Protocol::S_LOAD_EQUIPMENT& pkt)
{
	return true;
}
bool Handle_S_CONSUME_RESULT(PacketSessionRef& session, Protocol::S_CONSUME_RESULT& pkt)
{
	return true;
}
bool Handle_S_DROP_RESULT(PacketSessionRef& session, Protocol::S_DROP_RESULT& pkt)
{
	return true;
}
bool Handle_S_MOVE_INVENTORY_RESULT(PacketSessionRef& session, Protocol::S_MOVE_INVENTORY_RESULT& pkt)
{
	return true;
}
bool Handle_S_EQUIP_RESULT(PacketSessionRef& session, Protocol::S_EQUIP_RESULT& pkt)
{
	return true;
}
bool Handle_S_UNEQUIP_RESULT(PacketSessionRef& session, Protocol::S_UNEQUIP_RESULT& pkt)
{
	return true;
}
bool Handle_S_SWAP_ITEM(PacketSessionRef& session, Protocol::S_SWAP_ITEM& pkt)
{
	return true;
}
bool Handle_S_GOLD_CHANGE(PacketSessionRef& session, Protocol::S_GOLD_CHANGE& pkt)
{
	return true;
}
bool Handle_S_REMOVE_ITEM(PacketSessionRef& session, Protocol::S_REMOVE_ITEM& pkt) 
{
	return true;
}
bool Handle_S_RANKING(PacketSessionRef& session, Protocol::S_RANKING& pkt) 
{
	return true;
}