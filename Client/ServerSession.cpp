#include "pch.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"
#include <codecvt>

void ServerSession::OnConnected()
{
	Protocol::C_LOGIN pkt;

	pkt.set_name(Login());

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	ServerPacketHandler::HandlePacket(session, buffer, len);
}

void ServerSession::OnSend(int32 len)
{
	
}

void ServerSession::OnDisconnected()
{
	cout << "Disconnected" << endl;
}

void ServerSession::AttackPkt(uint64 _id, uint64 _skill)
{
	Protocol::C_ATTACK attackPkt;
	attackPkt.set_id(_id);
	attackPkt.set_skill(_skill);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(attackPkt);
	Send(sendBuffer);
}

void ServerSession::MovePkt(uint64 _direction, int64 _movetime)
{
	Protocol::C_MOVE movePkt;
	movePkt.set_direction(_direction);
	movePkt.set_move_time(_movetime);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(movePkt);
	Send(sendBuffer);
}

string ServerSession::Login()
{
	std::cout << "ID를 입력하시오 : " << std::endl;

	std::string input;
	std::cin >> input;

	return input;
}
