#pragma once
#include "DBProtocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_SD_LOGIN = 2000,
	PKT_SD_SAVE_PLAYER = 2001,
	PKT_SD_GET_INVENTORY = 2002,
	PKT_SD_UPDATE_INVENTORY = 2003,
	PKT_DS_LOGIN = 2004,
	PKT_DS_SAVE_PLAYER = 2005,
	PKT_DS_UPDATE_GOLD = 2006,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_SD_LOGIN(PacketSessionRef& session, DBProtocol::SD_LOGIN& pkt);
bool Handle_SD_SAVE_PLAYER(PacketSessionRef& session, DBProtocol::SD_SAVE_PLAYER& pkt);
bool Handle_SD_GET_INVENTORY(PacketSessionRef& session, DBProtocol::SD_GET_INVENTORY& pkt);
bool Handle_SD_UPDATE_INVENTORY(PacketSessionRef& session, DBProtocol::SD_UPDATE_INVENTORY& pkt);

class DBPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_SD_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_LOGIN>(Handle_SD_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_SD_SAVE_PLAYER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_SAVE_PLAYER>(Handle_SD_SAVE_PLAYER, session, buffer, len); };
		GPacketHandler[PKT_SD_GET_INVENTORY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_GET_INVENTORY>(Handle_SD_GET_INVENTORY, session, buffer, len); };
		GPacketHandler[PKT_SD_UPDATE_INVENTORY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_UPDATE_INVENTORY>(Handle_SD_UPDATE_INVENTORY, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_DS_LOGIN); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_SAVE_PLAYER& pkt) { return MakeSendBuffer(pkt, PKT_DS_SAVE_PLAYER); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_UPDATE_GOLD& pkt) { return MakeSendBuffer(pkt, PKT_DS_UPDATE_GOLD); }

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