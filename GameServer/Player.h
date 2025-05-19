#pragma once

#include "utils.h"
#include "GameSession.h"
#include "Room.h"
#include "Protocol.pb.h"
#include "StateSnap.h"

class Player
{
public:
	Player();
	~Player();
	Player(const string& _name, const STAT& _stat, const POS& _pos, S_STATE _state, uint32 _room, Protocol::PlayerType _pt); // NPC
	Player(const string& name, const STAT& stat, const POS& pos, S_STATE state, uint32 room, Protocol::PlayerType pt, uint64 gold, const vector<INVEN>& inventory, const vector<pair<Protocol::EquipmentSlot, ITEM_INFO>>& equipment);
public:
	void						LevelUp();

public:
	void SetId(uint64 _id) { WRITE_LOCK_IDX(0); myId = _id; }
	uint64 GetId() const { READ_LOCK_IDX(0); return myId; }

	void SetName(const string& _name) { WRITE_LOCK_IDX(0); myName = _name; }
	const string& GetName() const { READ_LOCK_IDX(0); return myName; }

	void SetStat(STAT _stat) { WRITE_LOCK_IDX(0); myStat = _stat; UpdateStatByEquipment(); }
	const STAT& GetStat() const { READ_LOCK_IDX(0); return myStat; }

	const USER_INFO GetPlayer() const;
	void SetStatLevel(int32 _level) { WRITE_LOCK_IDX(0); myStat.level = _level; }
	void SetStatHp(int32 _hp) { WRITE_LOCK_IDX(0); myStat.hp = _hp; }
	void SetStatMp(int32 _mp) { WRITE_LOCK_IDX(0); myStat.mp = _mp; }
	void SetStatExp(int32 _exp) { WRITE_LOCK_IDX(0); myStat.exp = _exp; }
	void SetStatMaxHp(int32 _maxHp) { WRITE_LOCK_IDX(0); myStat.maxHp = _maxHp; }
	void SetStatMaxMp(int32 _maxMp) { WRITE_LOCK_IDX(0); myStat.maxMp = _maxMp; }
	void SetStatMaxExp(int32 _maxExp) { WRITE_LOCK_IDX(0); myStat.maxExp = _maxExp; }
	void UpdateStatExp(int32 _exp) { WRITE_LOCK_IDX(0); myStat.exp += _exp; }
	void UpdateStatHp(int32 _hp) { WRITE_LOCK_IDX(0); myStat.hp += _hp; }
	void UpdateStatMp(int32 _mp) { WRITE_LOCK_IDX(0); myStat.mp += _mp; }

	void SetPos(POS _pos) { WRITE_LOCK_IDX(1); myPos = _pos; }
	const POS& GetPos() const { READ_LOCK_IDX(1); return myPos; }

	void SetState(S_STATE _state) { WRITE_LOCK_IDX(0); myState = _state; }
	const S_STATE& GetState() const { READ_LOCK_IDX(0); return myState; }

	void SetCurrentroom(uint32 _room) { WRITE_LOCK_IDX(0); currentRoom = _room; }
	uint32 GetCurrentroom() const { READ_LOCK_IDX(0); return currentRoom; }

	void SetPT(Protocol::PlayerType _pt) { WRITE_LOCK_IDX(0); PT = _pt; }
	Protocol::PlayerType GetPT() const { READ_LOCK_IDX(0); return PT; }

	void SetGold(uint64 _gold) { WRITE_LOCK_IDX(0); myGold = _gold; }
	uint64 GetGold() const { READ_LOCK_IDX(0); return myGold; }

	void SetOwnerSession(GameSessionRef& _session) { WRITE_LOCK_IDX(0); ownerSession = _session; }
	GameSessionRef& GetOwnerSession() { READ_LOCK_IDX(0); return ownerSession; }

	// 인벤토리 관련
	void AddInventoryItem(Protocol::InventoryTab tab, const INVEN& item) { WRITE_LOCK_IDX(2); myInven[{tab, item.slot_index}] = item; }
	bool RemoveInventoryItem(Protocol::InventoryTab tab, uint64 slotIndex) { WRITE_LOCK_IDX(2); return myInven.erase({ tab, slotIndex }) > 0; }
	const INVEN* GetInventoryItem(Protocol::InventoryTab tab, uint64 slotIndex) const {
		READ_LOCK_IDX(2);
		auto it = myInven.find({ tab, slotIndex });
		return it != myInven.end() ? &it->second : nullptr;
	}
	const InventoryMap& GetInventory() const { READ_LOCK_IDX(2); return myInven; }

	// 장비 관련
	void EquipItem(Protocol::EquipmentSlot slot, const ITEM_INFO& item) { WRITE_LOCK_IDX(2); myEquip[slot] = item; }
	bool UnequipItem(Protocol::EquipmentSlot slot) { WRITE_LOCK_IDX(2); return myEquip.erase(slot) > 0; }
	const ITEM_INFO* GetEquippedItem(Protocol::EquipmentSlot slot) const {
		READ_LOCK_IDX(2);
		auto it = myEquip.find(slot);
		return it != myEquip.end() ? &it->second : nullptr;
	}
	const EquipmentMap& GetEquipments() const { READ_LOCK_IDX(2); return myEquip; }

	void	UpdateStatByEquipment();

	StateSnap& GetStateSnap() { return snap; }
	void AddStateSnap(StateSnap::FLAG _flag, const StateSnap::StateSnapshot _snapShot);
	void	InitializeStateSnap() { snap.SetPlayerInfo(myId, myName); }

	void	ConsumeItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex);
	void	DropItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _quantity);
	void	MoveInventoryItem(Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex);
	void	InvenSwapItem(uint64 _fromItemId, Protocol::InventoryTab _fromTab, uint64 _fromIndex, uint64 _toItemId, Protocol::InventoryTab _toTab, uint64 _toIndex);
	void	AddItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _quantity);
	void	RemoveItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex);
	void	Equip(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotInex, Protocol::EquipmentSlot _slot);
	void	UnEquip(uint64 _itemId, uint64 _slotIndex, Protocol::EquipmentSlot _slot, Protocol::InventoryTab _tab);
	void	UpdateItem(Protocol::InventoryTab _tab, uint64 _itemId, uint64 _slotIndex, uint64 _quantity);

	uint64	GetEmptyIndex(Protocol::InventoryTab _tab);
	void	AddGold(int _gold);

private:
	mutable					USE_MANY_LOCKS(3); // 0 - id,name,stat,state,room,gold,PT , 1 - pos , 2 - inven , equip

private:
	uint64					myId;
	string					myName;
	POS						myPos;
	STAT					myStat;
	S_STATE					myState;
	uint64					myGold;
	uint32					currentRoom;
	Protocol::PlayerType	PT;

	GameSessionRef			ownerSession;
	
	InventoryMap			myInven;
	EquipmentMap			myEquip;
	StateSnap				snap;

public:
	atomic<bool>			active;
};

