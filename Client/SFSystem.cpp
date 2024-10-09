#pragma once
#include "pch.h"
#include "SFSystem.h"
#include "Client.h"
#include "Player.h"
SFSystem* SFSystem::s_instance = nullptr;

SFSystem::SFSystem()
{
	text.resize(static_cast<int>(SystemText::CHAT) + 1);
	box.resize(static_cast<int>(SystemBox::EXPINNERBOX) + 1);

	InitText();
	InitBox();

	board = new sf::Texture;
	player = new sf::Texture;
	monster = new sf::Texture;
	player_attack = new sf::Texture;

	ASSERT_CRASH(board->loadFromFile("./Image/maps.bmp"));
	ASSERT_CRASH(player->loadFromFile("./Image/Player.png"));
	ASSERT_CRASH(monster->loadFromFile("./Image/Monster.png"));
	ASSERT_CRASH(player_attack->loadFromFile("./Image/player_Attack1.png"));
	ASSERT_CRASH(font.loadFromFile("cour.ttf"));
}

SFSystem::~SFSystem()
{
	delete			board;
	delete			player;
	delete			monster;
	delete			player_attack;
}

SFSystem* SFSystem::GetInstance()
{
	if (s_instance == nullptr)
	{
		s_instance = new SFSystem();
	}
	return s_instance;
}

void SFSystem::InitText()
{
	for (int i = 0; i < text.size(); ++i)
	{
		text[i].setFont(font);
		text[i].setFillColor(sf::Color(WHITE));
		text[i].setStyle(sf::Text::Bold);
	}


	text[SystemText::HP].setPosition(1050, 450);
	text[SystemText::MP].setPosition(1050, 550);
	text[SystemText::MAXHP].setPosition(1200, 450);
	text[SystemText::MAXMP].setPosition(1200, 550);
	text[SystemText::EXP].setPosition(1050, 650);
	text[SystemText::MAXEXP].setPosition(1200, 650);
	text[SystemText::LEVEL].setPosition(1050, 350);
	text[SystemText::NAME].setPosition(0, 0);
	text[SystemText::PLAYERPOS].setPosition(0, 0);

}

void SFSystem::InitBox()
{
	for (int i = 0; i < 3; ++i)
	{
		box[i].setSize(sf::Vector2f(BOX));
		box[i].setFillColor(sf::Color(WHITE));
		box[i].setPosition(sf::Vector2f(INFORMATION_X, INFORMATION_HP_Y + i * 100));
		box[i].setOutlineThickness(OUTLINETHICK);
		box[i].setOutlineColor(sf::Color::Green);
	}

	for (int i = 3; i < 6; ++i)
		box[i].setPosition(sf::Vector2f(INFORMATION_X, INFORMATION_HP_Y + (i - 3) * 100));

	box[SystemBox::HPINNERBOX].setFillColor(sf::Color(RED));
	box[SystemBox::MPINNERBOX].setFillColor(sf::Color(BLUE));
	box[SystemBox::EXPINNERBOX].setFillColor(sf::Color(MAGENTA));
}

void SFSystem::SetText(SystemText _st, const char _str[])
{
	text[_st].setString(_str);
}

void SFSystem::SetChat(const string& _mess)
{
	if (systemChat.size() >= 10)
		systemChat.erase(systemChat.begin());

	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color(WHITE));
	text.setStyle(sf::Text::Bold);
	text.setScale(sf::Vector2f(0.5f, 0.5f));
	text.setString(_mess);

	systemChat.push_back(text);

}

void SFSystem::SetInnerBoxSize(SystemBox _sb, int _inner, int _maxinner)
{
	box[_sb].setSize(sf::Vector2f(sf::Vector2f(((100.f * _inner / _maxinner) * 3), 30)));
}

void SFSystem::Update(ServerSessionRef& _session)
{
	ServerSessionRef session = _session;
	SetInnerBoxSize(SystemBox::HPINNERBOX, session->GetPlayer()->GetStat().hp, session->GetPlayer()->GetStat().maxHp);
	SetInnerBoxSize(SystemBox::MPINNERBOX, session->GetPlayer()->GetStat().mp, session->GetPlayer()->GetStat().maxMp);
	SetInnerBoxSize(SystemBox::EXPINNERBOX, session->GetPlayer()->GetStat().exp, session->GetPlayer()->GetStat().maxExp);

	string hp = to_string(session->GetPlayer()->GetStat().hp);
	string _hp = "HP : ";
	_hp += hp;

	SetText(SystemText::HP, _hp.c_str());

	string maxhp = to_string(session->GetPlayer()->GetStat().maxHp);
	string _maxhp = " / ";
	_maxhp += maxhp;

	SetText(SystemText::MAXHP, _maxhp.c_str());

	string mp = to_string(session->GetPlayer()->GetStat().mp);
	string _mp = "MP : ";
	_mp += mp;
	SetText(SystemText::MP, _mp.c_str());

	string maxmp = to_string(session->GetPlayer()->GetStat().maxMp);
	string _maxmp = " / ";
	_maxmp += maxmp;
	SetText(SystemText::MAXMP, _maxmp.c_str());

	string lv = to_string(session->GetPlayer()->GetStat().level);
	string _lv = "LV : ";
	_lv += lv;
	SetText(SystemText::LEVEL, _lv.c_str());

	string exp = to_string(session->GetPlayer()->GetStat().exp);
	string _exp = "EXP : ";
	_exp += exp;
	SetText(SystemText::EXP, _exp.c_str());

	string maxexp = to_string(session->GetPlayer()->GetStat().maxExp);
	string _maxexp = " / ";
	_maxexp += maxexp;
	SetText(SystemText::MAXEXP, _maxexp.c_str());

	session->GetPlayer()->Draw();
	for (auto& pl : session->GetClients())
		pl.second->Draw();

	Draw();
}

void SFSystem::Draw()
{
	for (int i = 0; i < text.size(); ++i)
		window->draw(text[i]);

	for (int i = 0; i < box.size(); ++i)
		window->draw(box[i]);

	for (int i = 0; i < systemChat.size(); ++i)
	{
		systemChat[i].setPosition(1040, 35 * i);
		window->draw(systemChat[i]);
	}

}



