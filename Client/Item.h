#pragma once
#include "SFSystem.h"

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
	void InitItemICons();
	
	bool IsWeaponType(uint32_t itemId) const;
	bool IsHelmentType(uint32_t itemId) const;
	bool IsTopType(uint32_t itemId) const;
	bool IsBottomType(uint32_t itemId) const;

	const unordered_map<int, pair<sf::Texture*, sf::IntRect>> GetIconMap() const { return itemIconMap; }
private:
	Item() = default;
	~Item() {};
	Item(const Item&) = delete;
	Item& operator=(const Item&) = delete;

	unordered_map<int, ITEM_INFO> item_map;

	unordered_map<string, sf::Texture> iconTextureCache;
	unordered_map<int, pair<sf::Texture*, sf::IntRect>> itemIconMap;
};

