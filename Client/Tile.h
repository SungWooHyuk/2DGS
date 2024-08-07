#pragma once
#include "SFSystem.h"

class Tile
{
private:
	Tile();
	static	Tile* o_instance;
	Tile(Tile* _tile, sf::Texture& t, int _x, int _y, int _x2, int _y2);

public:
	~Tile();
	static Tile* GetInstance();


	void	TileDraw();
	void	Draw(Tile* _tile);
	void	Move(Tile* _tile, int _x, int _y);

private:
	Tile*		platTile;
	Tile*		obstacleTile;
	Tile*		blueTownTile;
	Tile*		greyTownTile;
	
private:
	sf::Sprite	tileSprite;
};

