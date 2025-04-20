#pragma once
#include "pch.h"
#include "MapData.h"

MapData::MapData()
{
    InitMapSetting("mapdata.txt");
    InitTownMapSetting("townmap.txt");
}

void MapData::InitMapSetting(const char* _worldMapFilename)
{
	char data;
	FILE* fp = fopen(_worldMapFilename, "r");
	ASSERT_CRASH(fp != nullptr);

	int cnt = 0;

	while (fscanf(fp, "%c", &data) != EOF)
	{
		switch (data)
		{
		case '0':
			worldMap[cnt / W_WIDTH][cnt % W_HEIGHT] = e_PLAT;
			cnt++;
			break;
		case '3':
			worldMap[cnt / W_WIDTH][cnt % W_HEIGHT] = e_OBSTACLE;
			cnt++;
			break;
		}
	}

	fclose(fp);
}

void MapData::InitTownMapSetting(const char* _townMapFilename)
{

    FILE* fp_townMap = fopen(_townMapFilename, "r");
    ASSERT_CRASH(fp_townMap != nullptr);

    char data;
    int townStartX = W_WIDTH / 2;
    int townStartY = W_HEIGHT / 2;

    for (int i = 0; i < TOWN_SIZE; ++i)
    {
        for (int j = 0; j < TOWN_SIZE; ++j)
        {
            ASSERT_CRASH(fscanf(fp_townMap, " %c", &data) != EOF);

            switch (data)
            {
            case '0':
                worldMap[townStartX + i][townStartY + j] = e_PLAT;
                break;
            case '1':
                worldMap[townStartX + i][townStartY + j] = e_BTOWN;
                break;
            case '2':
                worldMap[townStartX + i][townStartY + j] = e_GTOWN;
                break;
            case '3':
                worldMap[townStartX + i][townStartY + j] = e_OBSTACLE;
                break;
            }
        }
    }

    fclose(fp_townMap);
}

char MapData::GetTile(int _x, int _y)
{
    return worldMap[_x][_y];
}
