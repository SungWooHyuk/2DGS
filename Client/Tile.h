#pragma once
#include "SFSystem.h"

class Tile
{
public:
	
	static Tile& GetInstance()
	{
		static Tile instance;
		return instance;
	}

	void	Init();
	void	TileDraw();
	
	void	Draw(sf::Sprite& sprite);
	void	Move(sf::Sprite& sprite, int _x, int _y);

private:
	~Tile();
	Tile();
	Tile(const Tile&) = delete;
	Tile& operator=(const Tile&) = delete;

private:
	sf::Sprite platSprite;
	sf::Sprite obstacleSprite;
	sf::Sprite blueTownSprite;
	sf::Sprite greyTownSprite;
};

