#pragma once
#include "utils.h"

class DropTable
{
public:
	static DropTable& GetInstance()
	{
		static DropTable instance;
		return instance;
	}

	bool LoadFromJson(const string& _filePath);
	const DROPTABLE* GetDropTable(uint32 _id) const;

private:
	DropTable() = default;
	~DropTable() {};
	DropTable(const DropTable&) = delete;
	DropTable& operator=(const DropTable&) = delete;

	unordered_map<int32, DROPTABLE> dropTableMap;
};

