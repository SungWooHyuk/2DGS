#include "pch.h"
#include "DBGameSession.h"
#include "DBGameSessionManager.h"
#include "GameDBPacketHandler.h"

void DBGameSession::OnConnected()
{
	DBMANAGER.SetSession(static_pointer_cast<DBGameSession>(shared_from_this()));
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
