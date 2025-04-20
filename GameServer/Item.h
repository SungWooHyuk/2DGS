#pragma once
#include "utils.h"

class Item
{
public:
	static Item& GetInstance()
	{
		static Item instance;
		return instance;
	}

	bool LoadFromJson(const string& _filePath);
	const ITEM_INFO* GetItem(int _itemid) const;

private:
	Item() = default;
	~Item() {};
	Item(const Item&) = delete;
	Item& operator=(const Item&) = delete;

	unordered_map<int, ITEM_INFO> item_map;
};

