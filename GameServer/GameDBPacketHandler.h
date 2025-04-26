#pragma once
#include "DBProtocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GDBPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_SD_LOGIN = 2000,
	PKT_SD_SAVE_PLAYER = 2001,
	PKT_SD_GET_INFOMATION = 2002,
	PKT_SD_EQUIP_ITEM = 2003,
	PKT_SD_FARMING_ITEM = 2004,
	PKT_SD_UNEQUIP_ITEM = 2005,
	PKT_SD_CONSUME_ITEM = 2006,
	PKT_SD_MOVE_ITEM = 2007,
	PKT_SD_REGISTER = 2008,
	PKT_SD_SAVE_INVENTORY = 2009,
	PKT_SD_SAVE_EQUIPMENT = 2010,
	PKT_SD_UPDATE_GOLD = 2011,
	PKT_DS_LOGIN = 2012,
	PKT_DS_REGISTER = 2013,
	PKT_DS_SAVE_RESULT = 2014,
	PKT_DS_USER_INFORMATION = 2015,
	PKT_DS_INVENTORY_INFORMATION = 2016,
	PKT_DS_EQUIP_INFORMATION = 2017,
	PKT_DS_EQUIP_ITEM = 2018,
	PKT_DS_FARMING_RESULT = 2019,
	PKT_DS_CONSUME_ITEM = 2020,
	PKT_DS_MOVE_RESULT = 2021,
	PKT_DS_UPDATE_GOLD = 2022,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_DS_LOGIN(PacketSessionRef& session, DBProtocol::DS_LOGIN& pkt);
bool Handle_DS_REGISTER(PacketSessionRef& session, DBProtocol::DS_REGISTER& pkt);
bool Handle_DS_SAVE_RESULT(PacketSessionRef& session, DBProtocol::DS_SAVE_RESULT& pkt);
bool Handle_DS_USER_INFORMATION(PacketSessionRef& session, DBProtocol::DS_USER_INFORMATION& pkt);
bool Handle_DS_INVENTORY_INFORMATION(PacketSessionRef& session, DBProtocol::DS_INVENTORY_INFORMATION& pkt);
bool Handle_DS_EQUIP_INFORMATION(PacketSessionRef& session, DBProtocol::DS_EQUIP_INFORMATION& pkt);
bool Handle_DS_EQUIP_ITEM(PacketSessionRef& session, DBProtocol::DS_EQUIP_ITEM& pkt);
bool Handle_DS_FARMING_RESULT(PacketSessionRef& session, DBProtocol::DS_FARMING_RESULT& pkt);
bool Handle_DS_CONSUME_ITEM(PacketSessionRef& session, DBProtocol::DS_CONSUME_ITEM& pkt);
bool Handle_DS_MOVE_RESULT(PacketSessionRef& session, DBProtocol::DS_MOVE_RESULT& pkt);
bool Handle_DS_UPDATE_GOLD(PacketSessionRef& session, DBProtocol::DS_UPDATE_GOLD& pkt);

class GameDBPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GDBPacketHandler[i] = Handle_INVALID;
		GDBPacketHandler[PKT_DS_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_LOGIN>(Handle_DS_LOGIN, session, buffer, len); };
		GDBPacketHandler[PKT_DS_REGISTER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_REGISTER>(Handle_DS_REGISTER, session, buffer, len); };
		GDBPacketHandler[PKT_DS_SAVE_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_SAVE_RESULT>(Handle_DS_SAVE_RESULT, session, buffer, len); };
		GDBPacketHandler[PKT_DS_USER_INFORMATION] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_USER_INFORMATION>(Handle_DS_USER_INFORMATION, session, buffer, len); };
		GDBPacketHandler[PKT_DS_INVENTORY_INFORMATION] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_INVENTORY_INFORMATION>(Handle_DS_INVENTORY_INFORMATION, session, buffer, len); };
		GDBPacketHandler[PKT_DS_EQUIP_INFORMATION] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_EQUIP_INFORMATION>(Handle_DS_EQUIP_INFORMATION, session, buffer, len); };
		GDBPacketHandler[PKT_DS_EQUIP_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_EQUIP_ITEM>(Handle_DS_EQUIP_ITEM, session, buffer, len); };
		GDBPacketHandler[PKT_DS_FARMING_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_FARMING_RESULT>(Handle_DS_FARMING_RESULT, session, buffer, len); };
		GDBPacketHandler[PKT_DS_CONSUME_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_CONSUME_ITEM>(Handle_DS_CONSUME_ITEM, session, buffer, len); };
		GDBPacketHandler[PKT_DS_MOVE_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_MOVE_RESULT>(Handle_DS_MOVE_RESULT, session, buffer, len); };
		GDBPacketHandler[PKT_DS_UPDATE_GOLD] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::DS_UPDATE_GOLD>(Handle_DS_UPDATE_GOLD, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GDBPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_SD_LOGIN); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_SAVE_PLAYER& pkt) { return MakeSendBuffer(pkt, PKT_SD_SAVE_PLAYER); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_GET_INFOMATION& pkt) { return MakeSendBuffer(pkt, PKT_SD_GET_INFOMATION); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_EQUIP_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_SD_EQUIP_ITEM); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_FARMING_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_SD_FARMING_ITEM); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_UNEQUIP_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_SD_UNEQUIP_ITEM); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_CONSUME_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_SD_CONSUME_ITEM); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_MOVE_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_SD_MOVE_ITEM); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_REGISTER& pkt) { return MakeSendBuffer(pkt, PKT_SD_REGISTER); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_SAVE_INVENTORY& pkt) { return MakeSendBuffer(pkt, PKT_SD_SAVE_INVENTORY); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_SAVE_EQUIPMENT& pkt) { return MakeSendBuffer(pkt, PKT_SD_SAVE_EQUIPMENT); }
	static SendBufferRef MakeSendBuffer(DBProtocol::SD_UPDATE_GOLD& pkt) { return MakeSendBuffer(pkt, PKT_SD_UPDATE_GOLD); }

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