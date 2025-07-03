#include "pch.h"
#include "Player.h"
#include "utils.h"
#include "RedisManager.h"
#include "StateSnap.h"
#include "Item.h"
#include "DBGameSessionManager.h"
#include "GameSessionManager.h"

Player::Player()
{
	myId = -1;
	myName = "NONAME";
	myState = ST_FREE;
	myPos = { -1, -1 };
	myStat = { -1, -1, -1, -1, -1, -1, -1 };
	active = false;
}

Player::~Player()
{
	cout << "~Player()" << endl;
}

Player::Player(const string& _name, const STAT& _stat, const POS& _pos, S_STATE _state, uint32 _room, Protocol::PlayerType _pt)
	:myName(_name), myStat(_stat), myPos(_pos), myState(_state), currentRoom(_room), PT(_pt)
{
	active = false;

	myGold = static_cast<int>(_pt) * 50;

}

Player::Player(const string& name, const STAT& stat, const POS& pos, S_STATE state, uint32 room, Protocol::PlayerType pt, uint64 gold, const vector<INVEN>& inventory, const vector<pair<Protocol::EquipmentSlot, ITEM_INFO>>& equipment)
	:myName(name), myStat(stat), myPos(pos), myState(state), currentRoom(room), PT(pt), myGold(gold)
{
	for (const auto& inv : inventory)
		myInven[{inv.tab_type, inv.slot_index}] = inv;

	for (const auto& eq : equipment)
		myEquip[eq.first] = eq.second;

	active = false;
	UpdateStatByEquipment();
	InitializeStateSnap();
}

void Player::LevelUp()
{
	WRITE_LOCK_IDX(0);
	int oldLevel = myStat.level;
	myStat.level++;
	myStat.exp = 0;
	myStat.maxExp *= 2;
	myStat.maxHp *= 2;
	myStat.maxMp *= 2;
	myStat.hp = myStat.maxHp;
	myStat.mp = myStat.maxMp;

	//GLogger::LogWithContext(spdlog::level::info, myName, "LevelUp",
	//	"Player leveled up from {} to {}. New stats - HP: {}/{}, MP: {}/{}, MaxExp: {}",
	//	oldLevel, myStat.level, myStat.hp, myStat.maxHp, myStat.mp, myStat.maxMp, myStat.maxExp);
}

const USER_INFO Player::GetPlayer() const
{
	USER_INFO player;
	
	READ_LOCK_IDX(0);
	READ_LOCK_IDX(1);

	player.name = myName;
	player.posx = myPos.posx;
	player.posy = myPos.posy;
	player.level = myStat.level;
	player.hp = myStat.hp;
	player.maxHp = myStat.maxHp;
	player.mp = myStat.mp;
	player.maxMp = myStat.maxMp;
	player.exp = myStat.exp;
	player.maxExp = myStat.maxExp;
	player.gold = myGold;

	return player;
}

void Player::UpdateStatByEquipment()
{
	myStat.attackPower = 0;
	myStat.defencePower = 0;
	myStat.magicPower = 0;
	myStat.strength = 0;

	for (const auto& [slot, item] : myEquip)
	{
		myStat.attackPower += item.equipmentInfo.attackPower;
		myStat.defencePower += item.equipmentInfo.defensePower;
		myStat.magicPower += item.equipmentInfo.magicPower;
		myStat.strength += item.equipmentInfo.strength;
	}
}

void Player::AddStateSnap(StateSnap::FLAG _flag, const StateSnap::StateSnapshot _snapShot)
{
	switch (_flag)
	{
	case StateSnap::INVENTORY_FLAG:
		snap.UpdateInventory(_snapShot.inventory);
		break;
	case StateSnap::EQUIPMENT_FLAG:
		snap.UpdateEquipment(_snapShot.equipment);
		break;
	case StateSnap::POS_FLAG:
		snap.UpdatePos(_snapShot.pos);
		break;
	case StateSnap::STATS_FLAG:
		snap.UpdateStats(_snapShot.stats);
		break;
	default:
		break;
	}
}

void Player::ConsumeItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex)
{
	//GLogger::LogWithContext(spdlog::level::info, myName, "ConsumeItem",
	//	"Consuming item ID: {} from tab: {}, slot: {}", _itemId, static_cast<int>(_tab), _slotIndex);

	WRITE_LOCK_IDX(2);

	InventoryKey key = { _tab, _slotIndex };

	auto it = myInven.find(key);
	if (it == myInven.end()) {
		GLogger::LogWithContext(spdlog::level::err, myName, "ConsumeItem", "it == myInven.end()");
		return;
	}

	INVEN& item = it->second;

	if (item.itemId != _itemId) {
		GLogger::LogWithContext(spdlog::level::err, myName, "ConsumeItem", "item.itemId != _itemId");
		return;
	}

	if (item.quantity <= 0) {
		GLogger::LogWithContext(spdlog::level::err, myName, "ConsumeItem", "item.quantity <= 0");
		return;
	}

	auto itemInfo = ITEM.GetItem(item.itemId);

	auto type = itemInfo->effectType;
	auto value = itemInfo->effectValue;

	if (type == static_cast<int>(E_EFFECT_TYPE::HP)) {
		if (myStat.hp + value <= myStat.maxHp)
			myStat.hp += value;
		else
			myStat.hp = myStat.maxHp;
	}
	else if (type == static_cast<int>(E_EFFECT_TYPE::MP)) {
		if (myStat.mp + value <= myStat.maxMp)
			myStat.mp += value;
		else
			myStat.mp = myStat.maxMp;
	}


	--item.quantity;

	if (item.quantity == 0) {
		myInven.erase(key);
		if (ownerSession)
			ownerSession->RemoveItemPkt(item.itemId, item.tab_type, item.slot_index);
	}
	else
	{
		if (ownerSession)
			ownerSession->ConsumeItemPkt(true, item.itemId, item.tab_type, item.slot_index, item.quantity);
	}

	if (ownerSession)
		ownerSession->StatChangePkt(myStat.level, myStat.hp, myStat.maxHp, myStat.mp, myStat.maxMp, myStat.exp, myStat.maxExp);

	snap.UpdateInventory(myInven);
}

void Player::DropItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _quantity)
{
	//GLogger::LogWithContext(spdlog::level::info, myName, "DropItem",
	//	"Dropping item ID: {}, quantity: {} from tab: {}, slot: {}",
	//	_itemId, _quantity, static_cast<int>(_tab), _slotIndex);
	WRITE_LOCK_IDX(2);

	InventoryKey key = { _tab, _slotIndex };
	auto it = myInven.find(key);
	if (it == myInven.end() || it->second.itemId != _itemId) return;

	INVEN& item = it->second;
	if (item.quantity < _quantity) return;

	item.quantity -= _quantity;


	if (item.quantity == 0) {
		myInven.erase(key); // 삭제
		if (ownerSession)
			ownerSession->RemoveItemPkt(item.itemId, item.tab_type, item.slot_index);
	}
	else
	{
		if (ownerSession)
			ownerSession->DropPkt(true, item.itemId, item.tab_type, item.slot_index, item.quantity); // 현재 수량 보내기 

	}

	snap.UpdateInventory(myInven);
}

void Player::MoveInventoryItem(Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex)
{
	WRITE_LOCK_IDX(2);

	InventoryKey fromKey = { _fromTab, _fromIndex };
	InventoryKey toKey = { _toTab, _toIndex };

	auto fromIt = myInven.find(fromKey);
	if (fromIt == myInven.end()) {
		GLogger::LogWithContext(spdlog::level::err, myName, "MoveInventoryItem", "fromIt == myInven.end()");
		return;
	}
	// 이동 후 삭제 만약 이동자리에 있었으면 swap 요청이 왔을 것. quantity는 추후 쪼개서 이동시킬때를 대비해서 만듦
	myInven[toKey] = fromIt->second;
	myInven[toKey].slot_index = toKey.second;
	myInven.erase(fromIt);

	if (ownerSession)
		ownerSession->MoveInventoryIndex(true, fromIt->second.tab_type, fromIt->second.slot_index, myInven[toKey].tab_type, myInven[toKey].slot_index, fromIt->second.itemId, myInven[toKey].quantity);
	snap.UpdateInventory(myInven);
}
void Player::InvenSwapItem(uint64 _fromItemId, Protocol::InventoryTab _fromTab, uint64 _fromIndex, uint64 _toItemId, Protocol::InventoryTab _toTab, uint64 _toIndex)
{
	WRITE_LOCK_IDX(2);

	InventoryKey fromKey = { _fromTab, _fromIndex };
	InventoryKey toKey = { _toTab, _toIndex };

	auto fromIt = myInven.find(fromKey);
	auto toIt = myInven.find(toKey);
	if (fromIt == myInven.end() || toIt == myInven.end()) {
		GLogger::LogWithContext(spdlog::level::err, myName, "InvenSwapItem", "fromIt == myInven.end() || toIt == myInven.end()");
		return;
	}
	if (fromIt->second.itemId != _fromItemId || toIt->second.itemId != _toItemId) {
		GLogger::LogWithContext(spdlog::level::err, myName, "InvenSwapItem", "fromIt->second.itemId != _fromItemId || toIt->second.itemId != _toItemId");
		return;
	}
	swap(myInven[fromKey], myInven[toKey]);
	swap(myInven[fromKey].slot_index, myInven[toKey].slot_index);

	if (ownerSession)
		ownerSession->SwapPkt(true, myInven[fromKey].tab_type, myInven[fromKey].slot_index, myInven[toKey].tab_type, myInven[toKey].slot_index);

	snap.UpdateInventory(myInven);
}
void Player::AddItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _quantity)
{
	GLogger::LogWithContext(spdlog::level::info, myName, "AddItem",
		"Adding item ID: {}, quantity: {} to tab: {}, slot: {}",
		_itemId, _quantity, static_cast<int>(_tab), _slotIndex);

	WRITE_LOCK_IDX(2);

	const uint64 MAX_STACK = (_tab == Protocol::CONSUME) ? MAXCONSUME : (_tab == Protocol::MISC) ? MAXMISC : 1;

	uint64 remain = _quantity;

	for (auto& [key, item] : myInven)
	{
		if (key.first == _tab && item.itemId == _itemId && item.quantity < MAX_STACK)
		{
			uint64 space = MAX_STACK - item.quantity; 
			uint64 toAdd = min(remain, space);
			item.quantity += toAdd;
			remain -= toAdd;

			if (remain == 0)
				break;
		}
	}

	if (remain > 0)
	{
		InventoryKey key = { _tab, _slotIndex };
		auto& item = myInven[key];

		if (item.itemId == 0)
		{
			item.itemId = _itemId;
			item.quantity = min(remain, MAX_STACK);
			item.tab_type = _tab;
			item.slot_index = _slotIndex;
			remain -= item.quantity;
		}
		else if (item.itemId == _itemId && item.quantity < MAX_STACK)
		{
			uint64 space = MAX_STACK - item.quantity;
			uint64 toAdd = min(remain, space);
			item.quantity += toAdd;
			remain -= toAdd;
		}
	}

	if (ownerSession)
		ownerSession->LoadInventoryPkt(); // 추가 패킷 민들어서 교체하기
	snap.UpdateInventory(myInven);

}
void Player::RemoveItem(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex)
{
	GLogger::LogWithContext(spdlog::level::info, myName, "RemoveItem",
		"Adding item ID: {}, tab: {}, slot: {}",
		_itemId, static_cast<int>(_tab), _slotIndex);
	WRITE_LOCK_IDX(2);

	InventoryKey key = { _tab, _slotIndex };
	auto it = myInven.find(key);
	if (it == myInven.end() || it->second.itemId != _itemId) {
		GLogger::LogWithContext(spdlog::level::err, myName, "RemoveItem", "it == myInven.end() || it->second.itemId != _itemId");
		return;
	}
	myInven.erase(it);

	if (ownerSession)
		ownerSession->RemoveItemPkt(it->second.itemId, it->second.tab_type, it->second.slot_index);
	snap.UpdateInventory(myInven);
}
void Player::Equip(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, Protocol::EquipmentSlot _slot)
{
	WRITE_LOCK_IDX(2);

	InventoryKey key = { _tab, _slotIndex };
	auto it = myInven.find(key);
	if (it == myInven.end() || it->second.itemId != _itemId) return;

	const ITEM_INFO* info = ITEM.GetItem(it->second.itemId);
	if (info != nullptr)
		myEquip[_slot] = *info;
	else
		return;

	myInven.erase(it); // 인벤에서 삭제

	if (ownerSession) { // 패킷 합친 버전을 만드는게 더 나을듯? 
		ownerSession->RemoveItemPkt(it->second.itemId, it->second.tab_type, it->second.slot_index);
		ownerSession->EquipPkt(true, info->itemId, _slot);
	}

	snap.UpdateInventory(myInven);
	snap.UpdateEquipment(myEquip);
}
void Player::UnEquip(uint64 _itemId, uint64 _slotIndex, Protocol::EquipmentSlot _slot, Protocol::InventoryTab _tab)
{
	WRITE_LOCK_IDX(2);

	auto it = myEquip.find(_slot);
	if (it == myEquip.end() || it->second.itemId != _itemId) {
		GLogger::LogWithContext(spdlog::level::err, myName, "UnEquip", "it == myEquip.end() || it->second.itemId != _itemId");
		return;
	}
	InventoryKey key = { _tab, _slotIndex };
	INVEN invenItem;
	invenItem.itemId = _itemId;
	invenItem.tab_type = Protocol::EQUIP;
	invenItem.quantity = 1;
	invenItem.slot_index = _slotIndex;

	myInven[key] = invenItem;
	myEquip.erase(it);


	if (ownerSession) { // 패킷 합친 버전을 만드는게 더 나을듯? 아니면 그냥 아 Protocol::InventorySlot으로 한번에 처리하는게 더 깔끔할듯
		ownerSession->UnEquipPkt(true, it->second.itemId, _slot, myInven[key].tab_type, myInven[key].slot_index);
	}

	snap.UpdateInventory(myInven);
	snap.UpdateEquipment(myEquip);
}

void Player::UpdateItem(Protocol::InventoryTab _tab, uint64 _itemId, uint64 _slotIndex, uint64 _quantity)
{
	GLogger::LogWithContext(spdlog::level::info, myName, "UpdateItem",
		"Adding item ID: {}, tab: {}, slot: {} quantity: {}",
		_itemId, static_cast<int>(_tab), _slotIndex, _quantity);
	WRITE_LOCK_IDX(2);

	InventoryKey key = { _tab, _slotIndex };
	auto it = myInven.find(key);
	if (it == myInven.end()) {
		GLogger::LogWithContext(spdlog::level::err, myName, "UpdateItem", "it == myInven.end()");
		return;
	}
	if (it->second.itemId != _itemId) {
		GLogger::LogWithContext(spdlog::level::err, myName, "UpdateItem", "it->second.itemId != _itemId");
		return;
	}
	it->second.quantity = _quantity;

	if (it->second.quantity == 0) // 0 즉, 삭제될것
	{
		myInven.erase(it);
		if (ownerSession)
			ownerSession->RemoveItemPkt(it->second.itemId, it->second.tab_type, it->second.slot_index);
	}
	else
	{
		if (ownerSession) // 이거 Update pkt 만들어서 구분짓는게 나을듯 기능은 같음
			ownerSession->ConsumeItemPkt(true, it->second.itemId, it->second.tab_type, it->second.slot_index, it->second.quantity);
	}
	snap.UpdateInventory(myInven);
}

uint64 Player::GetEmptyIndex(Protocol::InventoryTab _tab)
{
	READ_LOCK_IDX(2);

	for (int i = 0; i < 25; ++i) // 25칸
	{
		pair<Protocol::InventoryTab, int> key = { _tab, i };
		if (myInven.find(key) == myInven.end())
			return i;
	}
	return -1;
}

void Player::AddGold(int _gold)
{
	WRITE_LOCK_IDX(0);

	int oldGold = myGold;
	myGold += _gold;
	GLogger::LogWithContext(spdlog::level::info, myName, "AddGold",
		"Gold changed from {} to {} (delta: {})", oldGold, myGold, _gold);

	if (ownerSession)
		ownerSession->GoldChangePkt(myGold, _gold);

	
	DBMANAGER.SendUpdateGoldPkt(myId, myName, myGold); // 골드는 즉시 저장
	GAMESESSIONMANAGER->UpdateGold(myName, myGold); // 레디스 새롭게 등록
}
