#include "pch.h"
#include "DropTable.h"

void from_json(const json& j, DROPINFO& item)
{ 
    item.itemId = j.at("itemId").get<int32>();
    item.minQuantity = j.at("minQuantity").get<int32>();
    item.maxQuantity = j.at("maxQuantity").get<int32>();
    item.dropRate = j.at("dropRate").get<float>();
}

bool DropTable::LoadFromJson(const string& _filePath)
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
        for (auto& [key, dropListJson] : data.items())
        {
            for (const auto& dropJson : dropListJson)
            {
                int dropTableId = stoi(key);
                DROPINFO drop = dropJson.get<DROPINFO>();
                dropTableMap[dropTableId].drops.push_back(drop);
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "JSON parsing fail during items: " << e.what() << endl;
        return false;
    }
    return true;
}

const DROPTABLE* DropTable::GetDropTable(uint32 _id) const
{
    auto it = dropTableMap.find(_id);
    if (it != dropTableMap.end())
        return &it->second;
    return nullptr;
}
