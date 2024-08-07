#pragma once
#include "pch.h"
#include "SFSystem.h"
#include "Client.h"

Client::Client()
{

}
Client::Client(Protocol::PlayerType pt, POS _pos, int _id, const string& _name)
{
	myId = _id;
	myName = _name;
	myPos = _pos;

	SetTexture(pt);
	SetNameColor(_name);
	Move(myPos.posx, myPos.posy);
};

Client::~Client()
{

};

void Client::SetTexture(Protocol::PlayerType pt)
{
	switch (pt)
	{
	case Protocol::PLAYER_TYPE_CLIENT:
		Sprite.setTexture(*SFSYSTEM->GetPlayer());
		Sprite.setTextureRect(sf::IntRect(CLIENT));
		break;
	case Protocol::PLAYER_TYPE_LV1:
		Sprite.setTexture(*SFSYSTEM->GetMonster());
		Sprite.setTextureRect(sf::IntRect(LV1MONSTER));
		break;
	case Protocol::PLAYER_TYPE_LV2:
		Sprite.setTexture(*SFSYSTEM->GetMonster());
		Sprite.setTextureRect(sf::IntRect(LV2MONSTER));
		break;
	case Protocol::PLAYER_TYPE_LV3:
		Sprite.setTexture(*SFSYSTEM->GetMonster());
		Sprite.setTextureRect(sf::IntRect(LV3MONSTER));
		break;
	case Protocol::PLAYER_TYPE_LV4:
		Sprite.setTexture(*SFSYSTEM->GetMonster());
		Sprite.setTextureRect(sf::IntRect(LV4MONSTER));
		break;
	default:
		Sprite.setTexture(*SFSYSTEM->GetPlayer());
		Sprite.setTextureRect(sf::IntRect(CLIENT));
		break;
	}
}


void Client::Move(int _x, int _y)
{
	myPos.posx = _x;
	myPos.posy = _y;
}

void Client::Draw()
{
	float fX = (myPos.posx - g_left_x)	*	65.f + 1;
	float fY = (myPos.posy - g_top_y)	*	65.f + 1;
	
	auto size = myTextName.getGlobalBounds();

	Sprite.setPosition(fX, fY);
	myTextName.setPosition(fX + 32 - size.width / 2, fY - 10);

	SFSYSTEM->GetWindow()->draw(Sprite);
	SFSYSTEM->GetWindow()->draw(myTextName);
}

void Client::SetNameColor(const string& _name)
{
	myTextName.setFont(SFSYSTEM->GetFont());
	myTextName.setString(_name);

	myTextName.setStyle(sf::Text::Bold);
	if (myId < MAX_USER)
		myTextName.setFillColor(sf::Color(WHITE));
	else if(myId > MAX_USER && myId < MAX_NPC / 4)
		myTextName.setFillColor(sf::Color(MAGENTA));
	else if(myId >= MAX_NPC / 4 && myId < (MAX_NPC / 4 * 2))
		myTextName.setFillColor(sf::Color(YELLOW));
	else if ((myId >= (MAX_NPC / 4 * 2) && myId < (MAX_NPC / 4 * 3)))
		myTextName.setFillColor(sf::Color(WHITE));
	else
		myTextName.setFillColor(sf::Color(GREEN));

}

