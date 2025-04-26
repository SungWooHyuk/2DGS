#pragma once
#include "pch.h"
#include "Tile.h"
#include "MapData.h"

Tile::Tile()
{
}

Tile::~Tile()
{
}

void Tile::Init()
{
	platSprite.setTexture(*SFSYSTEM.GetBoard());
	platSprite.setTextureRect(sf::IntRect(130, 0, TILE_WIDTH, TILE_WIDTH));

	obstacleSprite.setTexture(*SFSYSTEM.GetBoard());
	obstacleSprite.setTextureRect(sf::IntRect(195, 0, TILE_WIDTH, TILE_WIDTH));

	blueTownSprite.setTexture(*SFSYSTEM.GetBoard());
	blueTownSprite.setTextureRect(sf::IntRect(65, 0, TILE_WIDTH, TILE_WIDTH));

	greyTownSprite.setTexture(*SFSYSTEM.GetBoard());
	greyTownSprite.setTextureRect(sf::IntRect(0, 0, TILE_WIDTH, TILE_WIDTH));
}

void Tile::TileDraw()
{
	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;

			if (MAPDATA.GetTile(tile_y, tile_x) == MAPDATA.MAP_TYPE::e_PLAT) {
				Move(platSprite, TILE_WIDTH * i, TILE_WIDTH * j);
				Draw(platSprite);
			}
			else if (MAPDATA.GetTile(tile_y, tile_x) == MAPDATA.MAP_TYPE::e_OBSTACLE)
			{
				Move(obstacleSprite, TILE_WIDTH * i, TILE_WIDTH * j);
				Draw(obstacleSprite);
			}
			else if (MAPDATA.GetTile(tile_y, tile_x) == MAPDATA.MAP_TYPE::e_BTOWN)
			{
				Move(blueTownSprite, TILE_WIDTH * i, TILE_WIDTH * j);
				Draw(blueTownSprite);
			}
			else if (MAPDATA.GetTile(tile_y, tile_x) == MAPDATA.MAP_TYPE::e_GTOWN)
			{
				Move(greyTownSprite, TILE_WIDTH * i, TILE_WIDTH * j);
				Draw(greyTownSprite);
			}
		}
}

void Tile::Draw(sf::Sprite& sprite)
{
	SFSYSTEM.GetWindow()->draw(sprite);
}

void Tile::Move(sf::Sprite& sprite, int _x, int _y)
{
	sprite.setPosition(static_cast<float>(_x), static_cast<float>(_y));
}
