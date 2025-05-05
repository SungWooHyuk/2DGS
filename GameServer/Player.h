#pragma once

#include "utils.h"
#include "GameSession.h"
#include "Room.h"
#include "Protocol.pb.h"

class Player
{
public:
	using InventoryMap = unordered_map<int32, INVEN>;		// slot_index -> item
	using EquipmentMap = unordered_map<Protocol::EquipmentSlot, ITEM_INFO>;		// equip_slot -> item

	Player();
	~Player();
	Player(const string& _name, const STAT& _stat, const POS& _pos, S_STATE _state, uint32 _room, Protocol::PlayerType _pt);
	Player(const string& name, const STAT& stat, const POS& pos, S_STATE state, uint32 room, Protocol::PlayerType pt, uint64 gold, const vector<INVEN>& inventory, const vector<pair<Protocol::EquipmentSlot, ITEM_INFO>>& equipment);
public:
	void					LevelUp();

public:
	void					SetId(uint64 _id) { myId = _id; };
	uint64					GetId() const { return myId; };
	void					SetName(string _name) { myName = _name; };
	const string&			GetName() const { return myName; };
	
	void					SetStat(STAT _stat) { myStat = _stat; UpdateStatByEquipment(); };
	const STAT&				GetStat() const { return myStat; };

	void					SetStatLevel(int32 _level) { myStat.level = _level; }
	void					SetStatHp(int32 _hp) { myStat.hp = _hp; }
	void					SetStatMp(int32 _mp) { myStat.mp = _mp; }
	void					SetStatExp(int32 _exp) { myStat.exp = _exp; }
	void					SetStatMaxHp(int32 _maxHp) { myStat.maxHp = _maxHp; }
	void					SetStatMaxMp(int32 _maxMp) { myStat.maxMp = _maxMp; }
	void					SetStatMaxExp(int32 _maxExp) { myStat.maxExp = _maxExp; }

	void					UpdateStatExp(int32 _exp) { myStat.exp += _exp; }
	void					UpdateStatHp(int32 _hp) { myStat.hp += _hp; }
	void					UpdateStatMp(int32 _mp) { myStat.mp += _mp; }

	void					SetPos(POS _pos) { myPos = _pos; };
	const POS&				GetPos() const { return myPos; };
	void					SetState(S_STATE _state) { myState = _state; };
	const S_STATE&			GetState() const { return myState; };
	void					SetCurrentroom(uint32 _room) { currentRoom = _room; };
	uint32					GetCurrentroom() const { return currentRoom; };
	void					SetPT(Protocol::PlayerType _pt) { PT = _pt; };
	Protocol::PlayerType	GetPT() const { return PT; };
	void					SetGold(uint64 _gold) { myGold = _gold; };
	const uint64			GetGold() const { return myGold; };

	void					SetOwnerSession(GameSessionRef& _session) { ownerSession = _session; };
	GameSessionRef&			GetOwnerSession() { return ownerSession; };

	// 인벤토리 관련
	void AddInventoryItem(const INVEN& _item) { myInven[_item.slot_index] = _item; }
	bool RemoveInventoryItem(int32 _slotIndex) { return myInven.erase(_slotIndex) > 0; }
	const INVEN* GetInventoryItem(int32 slotIndex) const {
		auto it = myInven.find(slotIndex);
		if (it != myInven.end())
			return &it->second;
		else
			return nullptr;
	}
	const InventoryMap& GetInventory() const { return myInven; }

	// 장비 관련
	void EquipItem(Protocol::EquipmentSlot slot, const ITEM_INFO& item) { myEquip[slot] = item;}
	bool UnequipItem(Protocol::EquipmentSlot slot) { return myEquip.erase(slot) > 0; }
	const ITEM_INFO* GetEquippedItem(Protocol::EquipmentSlot slot) const {
		auto it = myEquip.find(slot);
		if (it != myEquip.end())
			return &it->second;
		else
			return nullptr;
	}
	const EquipmentMap& GetEquipments() const { return myEquip; }

	void	UpdateStatByEquipment();

private:
	GameSessionRef ownerSession;

private:
	uint64					myId;
	string					myName;
	POS						myPos;
	STAT					myStat;
	S_STATE					myState;
	uint64					myGold;
	uint32					currentRoom;
	Protocol::PlayerType	PT;
	
	InventoryMap			myInven;
	EquipmentMap			myEquip;
public:
	atomic<bool>			active;
};

