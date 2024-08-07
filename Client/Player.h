#pragma once

#include "SFSystem.h"
#include "Client.h"

class Player : public Client
{
public:
	Player(sf::Texture& _attackT, sf::Texture& _playerT, int _x, int _y, int _x2, int _y2, STAT _st, POS _pos, TP _tp, int32 _id, const string& _name);
	~Player() {};

	TP		GetTP() const { return myTp; };
	void	SetAttackTime();

	void	SetStat(STAT _st);
	STAT	GetStat() const { return myStat; };
	

	void	Draw();
	bool	Respawn(uint64 _exp, uint64 _hp, uint64 _x, uint64 _y);
private:
	STAT		myStat;
	TP			myTp;
private:

	sf::Sprite	attackSprite;
};

