#include "pch.h"
#include "DummySession.h"
#include "TestPacketHandler.h"
#include "NetworkModule.h"
#include "DummyManager.h"

void DummySession::OnConnected()
{
	Protocol::C_LOGIN pkt;
	pkt.set_name(Login());
	auto sendBuffer = TestPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void DummySession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	TestPacketHandler::HandlePacket(session, buffer, len);
}

void DummySession::OnSend(int32 len)
{
}

void DummySession::OnDisconnected()
{
	DUMMYMANAGER.Remove(static_pointer_cast<DummySession>(shared_from_this()));
	client = nullptr;
	cout << "Disconnected" << endl;
}

void DummySession::MovePkt(uint64 _direction, uint64 _moveTime)
{
	Protocol::C_MOVE movePkt;
	movePkt.set_direction(_direction);
	movePkt.set_move_time(_moveTime);
	auto sendBuffer = TestPacketHandler::MakeSendBuffer(movePkt);
	Send(sendBuffer);
}

string DummySession::Login()
{
	return "dummy" + std::to_string(active_clients);
}
