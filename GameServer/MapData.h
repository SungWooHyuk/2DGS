#pragma once
#include "utils.h"

class MapData
{
private:
	MapData();
	~MapData();
	MapData(const MapData&) = delete;
	MapData& operator=(const MapData&) = delete;


public:
	enum MAP_TYPE
	{
		e_PLAT,
		e_OBSTACLE,
		e_BTOWN,
		e_GTOWN
	};

public:
	static MapData& GetInstance()
	{
		static MapData instance;
		return instance;
	}

	void			InitMapSetting(const char* _worldMapFilename);
	void			InitTownMapSetting(const char* _townMapFilename);
	char			GetTile(int _x, int _y);
	void			InitMAP();

private:
	char	worldMap[W_WIDTH][W_HEIGHT];
	char	townMap[TOWN_SIZE][TOWN_SIZE];

};

