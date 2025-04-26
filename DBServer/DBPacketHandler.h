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
bool Handle_SD_LOGIN(PacketSessionRef& session, DBProtocol::SD_LOGIN& pkt);
bool Handle_SD_SAVE_PLAYER(PacketSessionRef& session, DBProtocol::SD_SAVE_PLAYER& pkt);
bool Handle_SD_GET_INFOMATION(PacketSessionRef& session, DBProtocol::SD_GET_INFOMATION& pkt);
bool Handle_SD_EQUIP_ITEM(PacketSessionRef& session, DBProtocol::SD_EQUIP_ITEM& pkt);
bool Handle_SD_FARMING_ITEM(PacketSessionRef& session, DBProtocol::SD_FARMING_ITEM& pkt);
bool Handle_SD_UNEQUIP_ITEM(PacketSessionRef& session, DBProtocol::SD_UNEQUIP_ITEM& pkt);
bool Handle_SD_CONSUME_ITEM(PacketSessionRef& session, DBProtocol::SD_CONSUME_ITEM& pkt);
bool Handle_SD_MOVE_ITEM(PacketSessionRef& session, DBProtocol::SD_MOVE_ITEM& pkt);
bool Handle_SD_REGISTER(PacketSessionRef& session, DBProtocol::SD_REGISTER& pkt);
bool Handle_SD_SAVE_INVENTORY(PacketSessionRef& session, DBProtocol::SD_SAVE_INVENTORY& pkt);
bool Handle_SD_SAVE_EQUIPMENT(PacketSessionRef& session, DBProtocol::SD_SAVE_EQUIPMENT& pkt);
bool Handle_SD_UPDATE_GOLD(PacketSessionRef& session, DBProtocol::SD_UPDATE_GOLD& pkt);

class DBPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GDBPacketHandler[i] = Handle_INVALID;
		GDBPacketHandler[PKT_SD_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_LOGIN>(Handle_SD_LOGIN, session, buffer, len); };
		GDBPacketHandler[PKT_SD_SAVE_PLAYER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_SAVE_PLAYER>(Handle_SD_SAVE_PLAYER, session, buffer, len); };
		GDBPacketHandler[PKT_SD_GET_INFOMATION] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_GET_INFOMATION>(Handle_SD_GET_INFOMATION, session, buffer, len); };
		GDBPacketHandler[PKT_SD_EQUIP_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_EQUIP_ITEM>(Handle_SD_EQUIP_ITEM, session, buffer, len); };
		GDBPacketHandler[PKT_SD_FARMING_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_FARMING_ITEM>(Handle_SD_FARMING_ITEM, session, buffer, len); };
		GDBPacketHandler[PKT_SD_UNEQUIP_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_UNEQUIP_ITEM>(Handle_SD_UNEQUIP_ITEM, session, buffer, len); };
		GDBPacketHandler[PKT_SD_CONSUME_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_CONSUME_ITEM>(Handle_SD_CONSUME_ITEM, session, buffer, len); };
		GDBPacketHandler[PKT_SD_MOVE_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_MOVE_ITEM>(Handle_SD_MOVE_ITEM, session, buffer, len); };
		GDBPacketHandler[PKT_SD_REGISTER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_REGISTER>(Handle_SD_REGISTER, session, buffer, len); };
		GDBPacketHandler[PKT_SD_SAVE_INVENTORY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_SAVE_INVENTORY>(Handle_SD_SAVE_INVENTORY, session, buffer, len); };
		GDBPacketHandler[PKT_SD_SAVE_EQUIPMENT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_SAVE_EQUIPMENT>(Handle_SD_SAVE_EQUIPMENT, session, buffer, len); };
		GDBPacketHandler[PKT_SD_UPDATE_GOLD] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<DBProtocol::SD_UPDATE_GOLD>(Handle_SD_UPDATE_GOLD, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GDBPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_DS_LOGIN); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_REGISTER& pkt) { return MakeSendBuffer(pkt, PKT_DS_REGISTER); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_SAVE_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_DS_SAVE_RESULT); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_USER_INFORMATION& pkt) { return MakeSendBuffer(pkt, PKT_DS_USER_INFORMATION); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_INVENTORY_INFORMATION& pkt) { return MakeSendBuffer(pkt, PKT_DS_INVENTORY_INFORMATION); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_EQUIP_INFORMATION& pkt) { return MakeSendBuffer(pkt, PKT_DS_EQUIP_INFORMATION); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_EQUIP_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_DS_EQUIP_ITEM); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_FARMING_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_DS_FARMING_RESULT); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_CONSUME_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_DS_CONSUME_ITEM); }
	static SendBufferRef MakeSendBuffer(DBProtocol::DS_MOVE_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_DS_MOVE_RESULT); }
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