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
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt);
bool Handle_S_LOAD_INVENTORY(PacketSessionRef& session, Protocol::S_LOAD_INVENTORY& pkt);
bool Handle_S_LOAD_EQUIPMENT(PacketSessionRef& session, Protocol::S_LOAD_EQUIPMENT& pkt);
bool Handle_S_CONSUME_RESULT(PacketSessionRef& session, Protocol::S_CONSUME_RESULT& pkt);
bool Handle_S_DROP_RESULT(PacketSessionRef& session, Protocol::S_DROP_RESULT& pkt);
bool Handle_S_MOVE_INVENTORY_RESULT(PacketSessionRef& session, Protocol::S_MOVE_INVENTORY_RESULT& pkt);
bool Handle_S_EQUIP_RESULT(PacketSessionRef& session, Protocol::S_EQUIP_RESULT& pkt);
bool Handle_S_UNEQUIP_RESULT(PacketSessionRef& session, Protocol::S_UNEQUIP_RESULT& pkt);
bool Handle_S_GOLD_CHANGE(PacketSessionRef& session, Protocol::S_GOLD_CHANGE& pkt);
bool Handle_S_ADD_OBJECT(PacketSessionRef& session, Protocol::S_ADD_OBJECT& pkt);
bool Handle_S_REMOVE_OBJECT(PacketSessionRef& session, Protocol::S_REMOVE_OBJECT& pkt);
bool Handle_S_MOVE_OBJECT(PacketSessionRef& session, Protocol::S_MOVE_OBJECT& pkt);
bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt);
bool Handle_S_STAT_CHANGE(PacketSessionRef& session, Protocol::S_STAT_CHANGE& pkt);
bool Handle_S_DAMAGE(PacketSessionRef& session, Protocol::S_DAMAGE& pkt);
bool Handle_S_RESPAWN(PacketSessionRef& session, Protocol::S_RESPAWN& pkt);
bool Handle_S_RANKING(PacketSessionRef& session, Protocol::S_RANKING& pkt);

class ServerPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOGIN>(Handle_S_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_S_LOAD_INVENTORY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOAD_INVENTORY>(Handle_S_LOAD_INVENTORY, session, buffer, len); };
		GPacketHandler[PKT_S_LOAD_EQUIPMENT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOAD_EQUIPMENT>(Handle_S_LOAD_EQUIPMENT, session, buffer, len); };
		GPacketHandler[PKT_S_CONSUME_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_CONSUME_RESULT>(Handle_S_CONSUME_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_DROP_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_DROP_RESULT>(Handle_S_DROP_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_MOVE_INVENTORY_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_MOVE_INVENTORY_RESULT>(Handle_S_MOVE_INVENTORY_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_EQUIP_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_EQUIP_RESULT>(Handle_S_EQUIP_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_UNEQUIP_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_UNEQUIP_RESULT>(Handle_S_UNEQUIP_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_GOLD_CHANGE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_GOLD_CHANGE>(Handle_S_GOLD_CHANGE, session, buffer, len); };
		GPacketHandler[PKT_S_ADD_OBJECT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ADD_OBJECT>(Handle_S_ADD_OBJECT, session, buffer, len); };
		GPacketHandler[PKT_S_REMOVE_OBJECT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_REMOVE_OBJECT>(Handle_S_REMOVE_OBJECT, session, buffer, len); };
		GPacketHandler[PKT_S_MOVE_OBJECT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_MOVE_OBJECT>(Handle_S_MOVE_OBJECT, session, buffer, len); };
		GPacketHandler[PKT_S_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_CHAT>(Handle_S_CHAT, session, buffer, len); };
		GPacketHandler[PKT_S_STAT_CHANGE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_STAT_CHANGE>(Handle_S_STAT_CHANGE, session, buffer, len); };
		GPacketHandler[PKT_S_DAMAGE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_DAMAGE>(Handle_S_DAMAGE, session, buffer, len); };
		GPacketHandler[PKT_S_RESPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_RESPAWN>(Handle_S_RESPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_RANKING] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_RANKING>(Handle_S_RANKING, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CONSUME_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_CONSUME_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_DROP_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_DROP_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE_INVENTORY_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE_INVENTORY_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_INVEN_SWAP_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_INVEN_SWAP_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ADD_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_ADD_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_REMOVE_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_REMOVE_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_EQUIP& pkt) { return MakeSendBuffer(pkt, PKT_C_EQUIP); }
	static SendBufferRef MakeSendBuffer(Protocol::C_UNEQUIP& pkt) { return MakeSendBuffer(pkt, PKT_C_UNEQUIP); }
	static SendBufferRef MakeSendBuffer(Protocol::C_SORT_INVENTORY& pkt) { return MakeSendBuffer(pkt, PKT_C_SORT_INVENTORY); }
	static SendBufferRef MakeSendBuffer(Protocol::C_UPDATE_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_UPDATE_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT); }
	static SendBufferRef MakeSendBuffer(Protocol::C_TELEPORT& pkt) { return MakeSendBuffer(pkt, PKT_C_TELEPORT); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ATTACK& pkt) { return MakeSendBuffer(pkt, PKT_C_ATTACK); }
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGOUT& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGOUT); }

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