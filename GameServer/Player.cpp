#include "pch.h"
#include "Player.h"
#include "utils.h"
#include "RedisManager.h"

Player::Player() 
{
	myId = -1;
	myName = "NONAME";
	myState = ST_FREE;
	myPos = { -1, -1 };
	myStat = { -1, -1, -1, -1, -1, -1, -1};
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
}

Player::Player(const string& name, const STAT& stat, const POS& pos, S_STATE state, uint32 room, Protocol::PlayerType pt, uint64 gold, const vector<INVEN>& inventory, const vector<pair<Protocol::EquipmentSlot, ITEM_INFO>>& equipment)
	:myName(name), myStat(stat), myPos(pos), myState(state), currentRoom(room), PT(pt), myGold(gold)
{
	for (const auto& inv : inventory)
		myInven[inv.slot_index] = inv;

	for (const auto& eq : equipment)
		myEquip[eq.first] = eq.second;

	active = false;
	UpdateStatByEquipment();
}

void Player::LevelUp()
{
	myStat.level++;
	myStat.exp = 0;
	myStat.maxExp *= 2;
	myStat.maxHp *= 2;
	myStat.maxMp *= 2;
	myStat.hp = myStat.maxHp;
	myStat.mp = myStat.maxMp;

	// Redis에 레벨 업데이트
	//RedisManager::GetInstance().UpdatePlayerLevel(myName, myStat.level);
}

void Player::UpdateStatByEquipment()
{
	myStat.attackPower = 0;
	myStat.defencePower = 0;
	myStat.magicPower= 0;
	myStat.strength = 0;

	for (const auto& [slot, item] : myEquip)
	{
		myStat.attackPower += item.equipmentInfo.attackPower;
		myStat.defencePower += item.equipmentInfo.defensePower;
		myStat.magicPower += item.equipmentInfo.magicPower;
		myStat.strength += item.equipmentInfo.strength;
	}
}

