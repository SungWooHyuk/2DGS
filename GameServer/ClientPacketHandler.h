#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_C_CONSUME_ITEM = 1001,
	PKT_C_DROP_ITEM = 1002,
	PKT_C_MOVE_INVENTORY_ITEM = 1003,
	PKT_C_INVEN_SWAP_ITEM = 1004,
	PKT_C_ADD_ITEM = 1005,
	PKT_C_REMOVE_ITEM = 1006,
	PKT_C_EQUIP = 1007,
	PKT_C_UNEQUIP = 1008,
	PKT_C_SORT_INVENTORY = 1009,
	PKT_C_UPDATE_ITEM = 1010,
	PKT_C_MOVE = 1011,
	PKT_C_CHAT = 1012,
	PKT_C_TELEPORT = 1013,
	PKT_C_ATTACK = 1014,
	PKT_C_LOGOUT = 1015,
	PKT_S_LOGIN = 1016,
	PKT_S_LOAD_INVENTORY = 1017,
	PKT_S_LOAD_EQUIPMENT = 1018,
	PKT_S_CONSUME_RESULT = 1019,
	PKT_S_DROP_RESULT = 1020,
	PKT_S_MOVE_INVENTORY_RESULT = 1021,
	PKT_S_EQUIP_RESULT = 1022,
	PKT_S_UNEQUIP_RESULT = 1023,
	PKT_S_GOLD_CHANGE = 1024,
	PKT_S_ADD_OBJECT = 1025,
	PKT_S_REMOVE_OBJECT = 1026,
	PKT_S_MOVE_OBJECT = 1027,
	PKT_S_CHAT = 1028,
	PKT_S_STAT_CHANGE = 1029,
	PKT_S_DAMAGE = 1030,
	PKT_S_RESPAWN = 1031,
	PKT_S_RANKING = 1032,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt);
bool Handle_C_CONSUME_ITEM(PacketSessionRef& session, Protocol::C_CONSUME_ITEM& pkt);
bool Handle_C_DROP_ITEM(PacketSessionRef& session, Protocol::C_DROP_ITEM& pkt);
bool Handle_C_MOVE_INVENTORY_ITEM(PacketSessionRef& session, Protocol::C_MOVE_INVENTORY_ITEM& pkt);
bool Handle_C_INVEN_SWAP_ITEM(PacketSessionRef& session, Protocol::C_INVEN_SWAP_ITEM& pkt);
bool Handle_C_ADD_ITEM(PacketSessionRef& session, Protocol::C_ADD_ITEM& pkt);
bool Handle_C_REMOVE_ITEM(PacketSessionRef& session, Protocol::C_REMOVE_ITEM& pkt);
bool Handle_C_EQUIP(PacketSessionRef& session, Protocol::C_EQUIP& pkt);
bool Handle_C_UNEQUIP(PacketSessionRef& session, Protocol::C_UNEQUIP& pkt);
bool Handle_C_SORT_INVENTORY(PacketSessionRef& session, Protocol::C_SORT_INVENTORY& pkt);
bool Handle_C_UPDATE_ITEM(PacketSessionRef& session, Protocol::C_UPDATE_ITEM& pkt);
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
		GPacketHandler[PKT_C_CONSUME_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CONSUME_ITEM>(Handle_C_CONSUME_ITEM, session, buffer, len); };
		GPacketHandler[PKT_C_DROP_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_DROP_ITEM>(Handle_C_DROP_ITEM, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE_INVENTORY_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MOVE_INVENTORY_ITEM>(Handle_C_MOVE_INVENTORY_ITEM, session, buffer, len); };
		GPacketHandler[PKT_C_INVEN_SWAP_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_INVEN_SWAP_ITEM>(Handle_C_INVEN_SWAP_ITEM, session, buffer, len); };
		GPacketHandler[PKT_C_ADD_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ADD_ITEM>(Handle_C_ADD_ITEM, session, buffer, len); };
		GPacketHandler[PKT_C_REMOVE_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_REMOVE_ITEM>(Handle_C_REMOVE_ITEM, session, buffer, len); };
		GPacketHandler[PKT_C_EQUIP] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_EQUIP>(Handle_C_EQUIP, session, buffer, len); };
		GPacketHandler[PKT_C_UNEQUIP] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_UNEQUIP>(Handle_C_UNEQUIP, session, buffer, len); };
		GPacketHandler[PKT_C_SORT_INVENTORY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_SORT_INVENTORY>(Handle_C_SORT_INVENTORY, session, buffer, len); };
		GPacketHandler[PKT_C_UPDATE_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_UPDATE_ITEM>(Handle_C_UPDATE_ITEM, session, buffer, len); };
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
	static SendBufferRef MakeSendBuffer(Protocol::S_LOAD_INVENTORY& pkt) { return MakeSendBuffer(pkt, PKT_S_LOAD_INVENTORY); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LOAD_EQUIPMENT& pkt) { return MakeSendBuffer(pkt, PKT_S_LOAD_EQUIPMENT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CONSUME_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_S_CONSUME_RESULT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DROP_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_S_DROP_RESULT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE_INVENTORY_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE_INVENTORY_RESULT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_EQUIP_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_S_EQUIP_RESULT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_UNEQUIP_RESULT& pkt) { return MakeSendBuffer(pkt, PKT_S_UNEQUIP_RESULT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_GOLD_CHANGE& pkt) { return MakeSendBuffer(pkt, PKT_S_GOLD_CHANGE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ADD_OBJECT& pkt) { return MakeSendBuffer(pkt, PKT_S_ADD_OBJECT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_REMOVE_OBJECT& pkt) { return MakeSendBuffer(pkt, PKT_S_REMOVE_OBJECT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE_OBJECT& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE_OBJECT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_STAT_CHANGE& pkt) { return MakeSendBuffer(pkt, PKT_S_STAT_CHANGE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DAMAGE& pkt) { return MakeSendBuffer(pkt, PKT_S_DAMAGE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_RESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_RESPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_RANKING& pkt) { return MakeSendBuffer(pkt, PKT_S_RANKING); }

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