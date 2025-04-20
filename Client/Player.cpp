#pragma once
#include "pch.h"
#include "Player.h"
#include "Client.h"

Player::Player(sf::Texture& _attackT, sf::Texture& _playerT, int _x, int _y, int _x2, int _y2, STAT _st, POS _pos, TP _tp, int32 _id, const string& _name)
	:Client(Protocol::PLAYER_TYPE_CLIENT, _pos, _id, _name)
{
	Sprite.setTexture(_playerT);
	Sprite.setTextureRect(sf::IntRect(_x, _y, _x2, _y2));

	attackSprite.setTexture(_attackT);
	attackSprite.setTextureRect(sf::IntRect(_x, _y, _x2, _y2));

	myTp = _tp;
	myStat = _st;

	Move(myPos.posx, myPos.posy);
}

void Player::SetAttackTime()
{
	myTp.attackEndTime = NOW + chrono::milliseconds(500);
	myTp.attackTime = NOW + chrono::milliseconds(1000);
}

void Player::SetStat(STAT _st)
{
	myStat.hp = _st.hp;
	myStat.maxHp = _st.maxHp;
	myStat.mp = _st.mp;
	myStat.maxMp = _st.maxMp;
	myStat.exp = _st.exp;
	myStat.maxExp = _st.maxExp;
	myStat.level = _st.level;
}

void Player::Draw()
{
	float fX = (myPos.posx - g_left_x) * 65.f + 1;
	float fY = (myPos.posy - g_top_y) * 65.f + 1;

	Sprite.setPosition(fX, fY);
	SFSYSTEM.GetWindow()->draw(Sprite);

	if (myTp.attackEndTime > NOW)
	{
		attackSprite.setPosition(fX, fY - 65);
		SFSYSTEM.GetWindow()->draw(attackSprite);
		attackSprite.setPosition(fX, fY + 65);
		SFSYSTEM.GetWindow()->draw(attackSprite);
		attackSprite.setPosition(fX + 65, fY);
		SFSYSTEM.GetWindow()->draw(attackSprite);
		attackSprite.setPosition(fX - 65, fY);
		SFSYSTEM.GetWindow()->draw(attackSprite);
	}
}

bool Player::Respawn(uint64 _exp, uint64 _hp, uint64 _x, uint64 _y)
{
	myStat.exp = _exp;
	myStat.hp = _hp;
	myPos.posx = _x;
	myPos.posy = _y;

	return true;
}
