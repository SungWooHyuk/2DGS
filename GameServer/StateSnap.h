#pragma once

#include "pch.h"
#include "utils.h"
#include "DBProtocol.pb.h"
#include "DBGameSessionManager.h"
#include "GLogger.h"
#include <bitset>

// 인벤토리, 장비창, 스탯, 위치 정보 저장 스냅관련

class StateSnap {
public:
	enum FLAG {
		INVENTORY_FLAG = 0,
		EQUIPMENT_FLAG,
		POS_FLAG,
		STATS_FLAG,
		DIRTY_FLAG_COUNT
	};

	struct StateSnapshot {
		InventoryMap inventory;
		EquipmentMap equipment;
		POS pos;
		STAT stats;
	};
private:
	USE_LOCK;
	StateSnapshot currentSnapshot;
	bitset<DIRTY_FLAG_COUNT> flags;
	int flagChangeCount = 0;
	uint64	id;
	string	name;

public:
	void SetPlayerInfo(uint64 _id, const string& _name) {
		WRITE_LOCK;
		id = _id;
		name = _name;
	}
	// 플래그 설정
	void UpdateInventory(const InventoryMap& _inv) {
		WRITE_LOCK;
		currentSnapshot.inventory = _inv;
		MarkFlag(INVENTORY_FLAG);
	}

	void UpdateEquipment(const EquipmentMap& _eq) {
		WRITE_LOCK;
		currentSnapshot.equipment = _eq;
		MarkFlag(EQUIPMENT_FLAG);
	}

	void UpdateStats(const STAT& _stats) {
		WRITE_LOCK;
		currentSnapshot.stats = _stats;
		MarkFlag(STATS_FLAG);
	}

	void UpdatePos(const POS& _pos) {
		WRITE_LOCK;
		currentSnapshot.pos = _pos;
		MarkFlag(POS_FLAG);
	}

	// DB 저장
	void SaveState() {
		WRITE_LOCK;
		if (flags.any() || flagChangeCount >= FORCE_SAVE_THRESHOLD) {
			if (flagChangeCount >= FORCE_SAVE_THRESHOLD) {
				GLogger::LogWithContext(spdlog::level::info, name, "SaveState FORCE_SAVE_THRESHOLD", "flagChangeCount >= FORCE_SAVE_THRESHOLD");
			}

			SaveToDatabase();
			flags.reset();
			flagChangeCount = 0;
		}
	}

private:
	void MarkFlag(FLAG _flag) {
		flags.set(static_cast<int>(_flag));
		flagChangeCount++;

		if (flagChangeCount >= FORCE_SAVE_THRESHOLD)
			SaveState();
	}

	void SaveToDatabase() {
		if (flags.test(INVENTORY_FLAG)) {
			GLogger::LogWithContext(spdlog::level::info, name, "SaveToDatabase", "Saving Inventory");
			DBMANAGER.SendInventoryPkt(id, name, currentSnapshot.inventory);
		}
		
		if (flags.test(EQUIPMENT_FLAG)) {
			GLogger::LogWithContext(spdlog::level::info, name, "SaveToDatabase", "Saving Equipment");
			DBMANAGER.SendEquipmentPkt(id, name, currentSnapshot.equipment);
		}
		
		if (flags.test(POS_FLAG) || flags.test(STATS_FLAG)) {
			GLogger::LogWithContext(spdlog::level::info, name, "SaveToDatabase", "Saving Player State (Pos/Stats)");
			DBMANAGER.SendPlayerStatePkt(id, name, currentSnapshot.pos, currentSnapshot.stats);
		}
	}
};
