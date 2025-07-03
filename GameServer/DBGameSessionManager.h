#pragma once
#include "DBGameSession.h"
#include "JobQueue.h"
#include "utils.h"

class DBSessionManager : public JobQueue
{
private:
	DBSessionManager() {}
	~DBSessionManager() {}
	DBSessionManager(const DBSessionManager&) = delete;
	DBSessionManager& operator=(const DBSessionManager&) = delete;

public:
	static DBSessionManager& GetInstance()
	{
		static DBSessionManager instance;
		return instance;
	}

public:
	void				SetSession(DBGameSessionRef session) { dbSession = session; }
	DBGameSessionRef	GetSession() { return dbSession; }
	void				RemoveSession() { dbSession = nullptr; }

	// 통합 패킷 전송 함수들
	void				SendInventoryPkt(uint64 id, const string& name, const InventoryMap& inventory);
	void				SendEquipmentPkt(uint64 id, const string& name, const EquipmentMap& equipment);
	void				SendPlayerStatePkt(uint64 id, const string& name, const POS& pos, const STAT& stats);
	void				SendUpdateGoldPkt(uint64 id, const string& name, int gold);
	void				SendSavePkt(uint64 id, const InventoryMap& inven, const EquipmentMap& equip, const USER_INFO& player);

private:
	DBGameSessionRef dbSession;
};