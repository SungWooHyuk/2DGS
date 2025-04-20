#include "pch.h"
#include "DBGameSession.h"
#include "GameDBPacketHandler.h"

void DBGameSession::OnConnected()
{
	cout << "DB Hello" << endl;
}

void DBGameSession::OnDisconnected()
{

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
