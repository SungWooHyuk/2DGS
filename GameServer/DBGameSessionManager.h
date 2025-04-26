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

private:
	USE_LOCK;
	DBGameSessionRef dbSession;
};