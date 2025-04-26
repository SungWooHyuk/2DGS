#pragma once

#include "SFSystem.h"
#include "Client.h"

class Player : public Client
{
public:
	Player(sf::Texture& _attackT, sf::Texture& _playerT, int _x, int _y, int _x2, int _y2, POS _pos, TP _tp, int32 _id, const string& _name, const uint64 _gold);
	~Player() {};

	TP		GetTP() const { return myTp; };
	void	SetAttackTime();

	void	SetStat(STAT _st);
	STAT	GetStat() const { return myStat; };
	

	void	Draw();
	bool	Respawn(uint64 _exp, uint64 _hp, uint64 _x, uint64 _y);

	void	SetInventory(const vector<INVEN>& _inv) { myInventory = _inv; }
	const	vector<INVEN>& GetInventory() const { return myInventory; }

	void	SetEquip(E_EQUIP _slot, uint32 _itemId) { myEquipments[_slot] = _itemId; }
	uint32	GetEquip(E_EQUIP slot) const
	{
		auto it = myEquipments.find(slot);
		return (it != myEquipments.end()) ? it->second : 0;
	}

	const	unordered_map<E_EQUIP, uint32>& GetEquipments() const { return myEquipments; }

private:
	STAT		myStat;
	TP			myTp;
	uint64		myGold;
	
	vector<INVEN> myInventory;
	unordered_map<E_EQUIP, uint32> myEquipments;
private:

	sf::Sprite	attackSprite;
};

