#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_C_MOVE = 1001,
	PKT_C_CHAT = 1002,
	PKT_C_TELEPORT = 1003,
	PKT_C_ATTACK = 1004,
	PKT_C_LOGOUT = 1005,
	PKT_S_LOGIN = 1006,
	PKT_S_ADD_OBJECT = 1007,
	PKT_S_REMOVE_OBJECT = 1008,
	PKT_S_MOVE_OBJECT = 1009,
	PKT_S_CHAT = 1010,
	PKT_S_STAT_CHANGE = 1011,
	PKT_S_DAMAGE = 1012,
	PKT_S_RESPAWN = 1013,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt);
bool Handle_C_TELEPORT(PacketSessionRef& session, Protocol::C_TELEPORT& pkt);
bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt);
bool Handle_C_LOGOUT(PacketSessionRef& session, Protocol::C_LOGOUT& pkt);

class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LOGIN>(Handle_C_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MOVE>(Handle_C_MOVE, session, buffer, len); };
		GPacketHandler[PKT_C_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CHAT>(Handle_C_CHAT, session, buffer, len); };
		GPacketHandler[PKT_C_TELEPORT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_TELEPORT>(Handle_C_TELEPORT, session, buffer, len); };
		GPacketHandler[PKT_C_ATTACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ATTACK>(Handle_C_ATTACK, session, buffer, len); };
		GPacketHandler[PKT_C_LOGOUT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LOGOUT>(Handle_C_LOGOUT, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ADD_OBJECT& pkt) { return MakeSendBuffer(pkt, PKT_S_ADD_OBJECT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_REMOVE_OBJECT& pkt) { return MakeSendBuffer(pkt, PKT_S_REMOVE_OBJECT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE_OBJECT& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE_OBJECT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_STAT_CHANGE& pkt) { return MakeSendBuffer(pkt, PKT_S_STAT_CHANGE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DAMAGE& pkt) { return MakeSendBuffer(pkt, PKT_S_DAMAGE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_RESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_RESPAWN); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};