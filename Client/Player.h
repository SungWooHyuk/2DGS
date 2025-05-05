#pragma once

#include "Client.h"
#include "SFSystem.h"
#include "pch.h"

class Player : public Client
{
public:
	Player(sf::Texture& _attackT, sf::Texture& _playerT, int _x, int _y, int _x2, int _y2, POS _pos, TP _tp, int32 _id, const string& _name, const uint64 _gold);
	~Player() {};

	void	SetOwnerSession(ServerSessionRef _session) { ownerSession = _session; }
	TP		GetTP() const { return myTp; };
	void	SetAttackTime();

	void	SetStat(STAT _st);
	STAT	GetStat() const { return myStat; };
	void	DamageHp(uint64 _damage) { myStat.hp -= _damage; };

	void	Draw();
	bool	Respawn(uint64 _exp, uint64 _hp, uint64 _x, uint64 _y);

	void	SetInventory(const vector<INVEN>& _inv) { myInventory = _inv; }
	const	vector<INVEN>& GetInventory() const { return myInventory; }
	void	AddItem(const INVEN& _item); 

	void	SetEquip(Protocol::EquipmentSlot _slot, uint32 _itemId);

	void	SetGold(uint64 _gold) { myGold = _gold; }
	const	uint64 GetGold() const { return myGold; }

	const	uint32	GetEquip(Protocol::EquipmentSlot _slot) const;
	const	unordered_map<Protocol::EquipmentSlot, uint32>& GetEquipments() const { return myEquipments; }

	bool	HasItem(Protocol::InventoryTab _tabType, uint64 _slotIndex) const;
	const	INVEN* GetItem(Protocol::InventoryTab _tabType, uint64 _slotIndex) const;

	// 인벤토리 관련 함수
	void	RemoveItem(Protocol::InventoryTab _tabType, uint64 _slotIndex);
	void	MoveItem(Protocol::InventoryTab _fromTab, uint64 _fromSlot, Protocol::InventoryTab _toTab, uint64 _toSlot, uint64 _quantity);
	void	SetEquipToInventory(uint64 _slotIndex, const INVEN& _item);
	void	SwapItems(Protocol::InventoryTab _tab1, uint64 _slot1, Protocol::InventoryTab _tab2, uint64 _slot2);
	void	UpdateItemQuantity(Protocol::InventoryTab _tabType, uint64 _slotIndex, uint64 _newQuantity);
	void	UnEquip(Protocol::EquipmentSlot _slot, uint64 _itemId);

	//pkt
	void	SendAddItemPkt(const INVEN& _inv); 
	void	SendEquipPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _inv_slot_index, Protocol::EquipmentSlot _slotIndex);
	void	SendUnEquipPkt(uint64 _itemId, uint64 _inv_slot_index, Protocol::InventoryTab _tabType, Protocol::EquipmentSlot _slotIndex);
	void	SendMoveInventoryItemPkt(Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex);
	void	SendConsumeItemPkt(uint64 _itemid, Protocol::InventoryTab _tabType, uint64 _slotIndex); 
	void	SendRemoveItemPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _slotIndex); 
	void	SendSwapItemPkt(uint64 _fromItemId, Protocol::InventoryTab _fromTab, uint64 _fromIndex, uint64 _toItemId, Protocol::InventoryTab _toTab, uint64 _toIndex);
	void	SendUpdateItemPkt(Protocol::InventoryTab _tabType, uint64 _itemId, uint64 _slotIndex, uint64 _quantity);
private:
	ServerSessionRef ownerSession;

	STAT		myStat;
	TP			myTp;
	uint64		myGold;
	vector<INVEN> myInventory;
	unordered_map<Protocol::EquipmentSlot, uint32> myEquipments;

private:
	sf::Sprite	attackSprite;
};

