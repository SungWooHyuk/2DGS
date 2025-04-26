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

Player::Player(string _name, STAT _stat, POS _pos, S_STATE _state, uint32 _room, Protocol::PlayerType _pt)
	:myName(_name), myStat(_stat), myPos(_pos), myState(_state), currentRoom(_room), PT(_pt)
{
	active = false;
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

