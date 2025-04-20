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

	void	Init(Tile* _tile, sf::Texture& t, int _x, int _y, int _x2, int _y2);
	void	TileDraw();
	
	void	Draw(Tile* _tile);
	void	Move(Tile* _tile, int _x, int _y);

private:
	~Tile();
	Tile();
	Tile(const Tile&) = delete;
	Tile& operator=(const Tile&) = delete;

private:
	Tile*		platTile;
	Tile*		obstacleTile;
	Tile*		blueTownTile;
	Tile*		greyTownTile;
	
private:
	sf::Sprite	tileSprite;
};

