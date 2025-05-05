#pragma once

#include "pch.h"
#include "Client.h"
#include "Player.h"

class ServerSession : public PacketSession
{
public:
	ServerSession() {};
	~ServerSession() { cout << "~ServerSession" << endl; };

	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;

    std::unordered_map<int, ClientRef>& GetClients() { return clients; }
    ClientRef& GetClients(int _id) { return clients[_id]; }
    void SetClients(std::unordered_map<int, ClientRef>& newClients) { clients = newClients; }
	void AddClient(int id, const ClientRef& client) { clients[id] = client; }

    const PlayerRef& GetPlayer() const { return player; }
    void SetPlayer(PlayerRef& _player) { player = _player; }

	void AttackPkt(uint64 _id, uint64 _skill);
	void MovePkt(uint64 _direction, int64 _movetime);

	void EquipPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _invSlotIndex, Protocol::EquipmentSlot _slotIndex);
	void UnEquipPkt(uint64 _itemId,uint64 _invSlotIndex, Protocol::InventoryTab _tabType, Protocol::EquipmentSlot _slotIndex);
	void MoveInventoryItemPkt(Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex);
	void ConsumeItemPkt(uint64 _itemid, Protocol::InventoryTab _tabType, uint64 _slotIndex);
	void RemoveItemPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _slotIndex);
	void AddItemPkt(const INVEN& _inv);
	void SwapItemPkt(uint64 _fromItemId, Protocol::InventoryTab _fromTab, uint64 _fromIndex, uint64 _toItemId, Protocol::InventoryTab _toTab, uint64 _toIndex);
	void UpdateItemPkt(Protocol::InventoryTab _tabType, uint64 _itemId, uint64 _slotIndex, uint64 _quantity);
public:
	string Login();
	
private:
	unordered_map<int, ClientRef> clients;
	PlayerRef player;
};