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
	void				SetSession(DBGameSessionRef session) { WRITE_LOCK; dbSession = session; }
	DBGameSessionRef	GetSession() { READ_LOCK; return dbSession; }
	void				RemoveSession() { WRITE_LOCK; dbSession = nullptr; }

private:
	USE_LOCK;
	DBGameSessionRef dbSession;
};