#include "pch.h"
#include "Item.h"

void from_json(const json& j, ITEM_INFO::EQUIPMENT_INFO& e)
{
    e.attackPower = j.value("attack_power", 0);
    e.defensePower = j.value("defence_power", 0); // defense Ω∫∆Á∏µ ¡÷¿«
    e.magicPower = j.value("magic_power", 0);
    e.strength = j.value("strength", 0);
}

void from_json(const json& j, ITEM_INFO& item)
{
    item.itemId = j.value("item_id", 0);
    item.name = j.value("name", "");
    item.itemType = static_cast<Protocol::InventoryTab>(j.value("item_type", 0));

    item.equipType = (j.contains("equip_type") && !j["equip_type"].is_null()) ? static_cast<Protocol::EquipmentSlot>(j["equip_type"].get<int32>()) : static_cast<Protocol::EquipmentSlot>(0);
    item.effectType = (j.contains("effect_type") && !j["effect_type"].is_null()) ? j["effect_type"].get<int32>() : 0;
    item.effectValue = (j.contains("effect_value") && !j["effect_value"].is_null()) ? j["effect_value"].get<int32>() : 0;

    item.requiredLevel = j.value("required_level", 0);
    item.description = j.value("description", "");

    if (j.contains("icon_path") && !j["icon_path"].is_null())
        item.iconPath = j["icon_path"].get<string>();
    else
        item.iconPath = "";

    if (j.contains("equipment_data") && !j["equipment_data"].is_null())
        j.at("equipment_data").get_to(item.equipmentInfo);
}

bool Item::LoadFromJson(const string& _filePath)
{
    ifstream in(_filePath);
    if (!in.is_open())
    {
        cerr << "file open fail: " << _filePath << endl;
        return false;
    }

    json data;
    try
    {
        data = json::parse(in);
    }
    catch (const exception& e)
    {
        cerr << "JSON parse error: " << e.what() << endl;
        return false;
    }

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
        cerr << "JSON parsing fail during items: " << e.what() << endl;
        return false;
    }

    cout << "Item JSON loaded successfully! (" << item_map.size() << " items)" << endl;
    return true;
}

const ITEM_INFO* Item::GetItem(int _itemid) const
{
    auto it = item_map.find(_itemid);
    if (it != item_map.end())
        return &it->second;
    return nullptr;
}
