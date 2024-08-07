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
			GDummyManager->Login(dummySession, cli.id(), cli.x(), cli.y());
			GDummyManager->Add(dummySession);
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

		GDummyManager->Move(serverSession, id, x, y, move_time);
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
