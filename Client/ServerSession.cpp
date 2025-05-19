#include "pch.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"
#include <codecvt>
#include "GLogger.h"

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
	Protocol::C_LOGOUT pkt;
	pkt.set_id(player->GetId());

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
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

void ServerSession::EquipPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _invSlotIndex, Protocol::EquipmentSlot _slotIndex)
{
	Protocol::C_EQUIP pkt;
	pkt.set_item_id(_itemId);
	pkt.set_tab_type(_tabType);
	pkt.set_slot_type(_slotIndex);
	pkt.set_inv_slot_index(_invSlotIndex);
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::UnEquipPkt(uint64 _itemId, uint64 _invSlotIndex, Protocol::InventoryTab _tabType, Protocol::EquipmentSlot _slotIndex)
{
	Protocol::C_UNEQUIP pkt;
	pkt.set_item_id(_itemId);
	pkt.set_slot_type(_slotIndex);
	pkt.set_inv_slot_index(_invSlotIndex);
	pkt.set_tab_type(_tabType);
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::MoveInventoryItemPkt(Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex)
{
	Protocol::C_MOVE_INVENTORY_ITEM pkt;
	pkt.set_inv_from_index(_fromIndex);
	pkt.set_from_tab(_fromTab);
	pkt.set_to_tab(_toTab);
	pkt.set_inv_to_index(_toIndex);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::ConsumeItemPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _slotIndex)
{
	Protocol::C_CONSUME_ITEM pkt;
	pkt.set_item_id(_itemId);
	pkt.set_tab_type(_tabType);
	pkt.set_inv_slot_index(_slotIndex);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::RemoveItemPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _slotIndex)
{
	Protocol::C_REMOVE_ITEM pkt;
	pkt.set_item_id(_itemId);
	pkt.set_tab_type(_tabType);
	pkt.set_inv_slot_index(_slotIndex);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::AddItemPkt(const INVEN& _inv)
{
	Protocol::C_ADD_ITEM pkt;
	pkt.set_item_id(_inv.itemId);
	pkt.set_tab_type(_inv.tab_type);
	pkt.set_inv_slot_index(_inv.slot_index);
	pkt.set_quantity(_inv.quantity);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::SwapItemPkt(uint64 _fromItemId, Protocol::InventoryTab _fromTab, uint64 _fromIndex, uint64 _toItemId, Protocol::InventoryTab _toTab, uint64 _toIndex)
{
	Protocol::C_INVEN_SWAP_ITEM pkt;
	pkt.set_from_item_id(_fromItemId);
	pkt.set_from_tab(_fromTab);
	pkt.set_inv_from_index(_fromIndex);
	pkt.set_to_item_id(_toItemId);
	pkt.set_to_tab(_toTab);
	pkt.set_inv_to_index(_toIndex);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

void ServerSession::UpdateItemPkt(Protocol::InventoryTab _tabType, uint64 _itemId, uint64 _slotIndex, uint64 _quantity)
{
	Protocol::C_UPDATE_ITEM pkt;
	pkt.set_tab_type(_tabType);
	pkt.set_item_id(_itemId);
	pkt.set_inv_slot_index(_slotIndex);
	pkt.set_quantity(_quantity);

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	Send(sendBuffer);
}

string ServerSession::Login()
{
	std::cout << "ID : " << std::endl;

	std::string input;
	std::cin >> input;

	return input;
}
