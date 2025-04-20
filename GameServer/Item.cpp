#include "pch.h"
#include "Item.h"

void from_json(const json& j, ITEM_INFO::EQUIPMENT_INFO& e)
{
    j.at("attackPower").get_to(e.attackPower);
    j.at("defensePower").get_to(e.defensePower);
    j.at("magicPower").get_to(e.magicPower);
    j.at("strength").get_to(e.strength);
}

void from_json(const json& j, ITEM_INFO& item)
{
    j.at("itemId").get_to(item.itemId);
    j.at("name").get_to(item.name);
    j.at("itemType").get_to(item.itemType);
    j.at("equipType").get_to(item.equipType);
    j.at("effectType").get_to(item.effectType);
    j.at("effectValue").get_to(item.effectValue);
    j.at("requiredLevel").get_to(item.requiredLevel);
    j.at("description").get_to(item.description);
    j.at("equipmentInfo").get_to(item.equipmentInfo);
}

bool Item::LoadFromJson(const string& _filePath)
{
	ifstream in("../Tools/TableToJson/items.json");
	//ifstream in(_filePath);
	if (!in.is_open())
	{
		cerr << "file open fail\n";
		return false;
	}

	json data = json::parse(in);
	
    try
    {
        for (const auto& item_json : data)
        {
            ITEM_INFO item = item_json.get<ITEM_INFO>();
            item_map[item.itemId] = item;
        }
    }
    catch (const exception& e)
    {
        cerr << "JSON parsing fail: " << e.what() << endl;
    }

	return true;
}

const ITEM_INFO* Item::GetItem(int _itemid) const
{
    auto it = item_map.find(_itemid);
    if (it != item_map.end())
        return &it->second;
    return nullptr;
}


