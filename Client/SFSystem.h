#pragma once
#include "pch.h"
#include "queue"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "ServerSession.h"

class SFSystem
{
public:
	static SFSystem& GetInstance()
	{
		static SFSystem instance;
		return instance;
	}
	
	void InitText();
	void InitBox();

	void SetText(SystemText _st, const char _str[]);
	void SetChat(const string& _mess);
	void SetInnerBoxSize(SystemBox _sb, int _inner, int _maxinner);
	void Update(ServerSessionRef& _session);
	void Draw();
	
    sf::RenderWindow* GetWindow() const { return window;}
    void SetWindow(sf::RenderWindow* _win) {window = _win;}

    sf::Font& GetFont() {return font;}
    void SetFont(const sf::Font& _f) {font = _f;}

    sf::Texture* GetMonster() const { return monster;}
    void SetMonster(sf::Texture* _tex) { monster = _tex;}

    sf::Texture* GetPlayer() const { return player;}
    void SetPlayer(sf::Texture* _tex) {player = _tex;}

    sf::Texture* GetPlayerAttack() const { return player_attack;}
    void SetPlayerAttack(sf::Texture* _tex) {player_attack = _tex;}

    sf::Texture* GetBoard() const {return board;}
    void SetBoard(sf::Texture* _tex) { board = _tex;}


public:
	sf::TcpSocket				socket;

private:
	SFSystem(); 
	~SFSystem();
	SFSystem(const SFSystem&) = delete;
	SFSystem& operator=(const SFSystem&) = delete;
private:
	USE_LOCK;
	vector<sf::Text>			text;
	vector<sf::RectangleShape>	box;
	vector<sf::Text>			systemChat;

private:
	sf::RenderWindow*			window;
	sf::Font					font;

private:
	sf::Texture*				monster;
	sf::Texture*				player;	
	sf::Texture*				player_attack;
	sf::Texture*				board;

};

