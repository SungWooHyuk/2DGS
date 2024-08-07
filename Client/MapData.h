#pragma once

class MapData 
{
private:
	static	MapData* m_instance;
	MapData();

public:
	enum MAP_TYPE
	{
		e_PLAT,
		e_OBSTACLE,
		e_BTOWN,
		e_GTOWN
	};

public:
	~MapData();
	static MapData* GetInstance();

	void InitMapSetting(const char* _worldMapFilename);
	void InitTownMapSetting(const char* _townMapFilename);
	char GetTile(int _x, int _y);

private:
	char	worldMap[W_WIDTH][W_HEIGHT];
	char	townMap[TOWN_SIZE][TOWN_SIZE];
};

