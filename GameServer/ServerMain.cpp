#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "GameDBPacketHandler.h"
#include "Job.h"
#include "Logger.h"
#include "GLogger.h"
#include <spdlog/spdlog.h>

#include "MapData.h"
#include "DBGameSession.h"
#include "DBGameSessionManager.h"

#include <concurrent_priority_queue.h>
#include <sqlext.h>
#include <locale>

#include "utils.h"

extern "C"
{
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}

#include "Item.h"
void DoGameWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;
		service->GetIocpCore()->Dispatch(10);
		ThreadManager::DistributeReservedJobs();
		ThreadManager::DoGlobalQueueWork();
	}
}

void DoDBWorkerJob(DBServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;
		service->GetIocpCore()->Dispatch(10);
		ThreadManager::DistributeReservedJobs();
		ThreadManager::DoGlobalQueueWork();
	}
}
int main()
{
	GLogger::Init("GameServer");
	MAPDATA.InitMAP();
	ClientPacketHandler::Init();
	GameDBPacketHandler::Init();
	GAMESESSIONMANAGER.InitializeNPC();
	ITEM.LoadFromJson("items.json");
	//GLogger::Log(spdlog::level::err, "Begin");

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 4000),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		MAX_USER
		);

	DBServiceRef dbservice = MakeShared<DBService>(
		NetAddress(L"127.0.0.1", DB_PORT),
		MakeShared<IocpCore>(),
		MakeShared<DBGameSession>,
		1
		);

	ASSERT_CRASH(dbservice->Start());
	ASSERT_CRASH(service->Start());

	int num_threads = std::thread::hardware_concurrency() - 3;
	int db_num_threads = 3;

	for (int32 i = 0; i < num_threads; ++i)
	{
		GThreadManager->Launch([&service]()
			{
				DoGameWorkerJob(service);
			});
	}

	// DB 서비스 스레드
	for (int32 i = 0; i < db_num_threads; ++i)
	{
		GThreadManager->Launch([&dbservice]()
			{
				DoDBWorkerJob(dbservice);
			});
	}


	DoGameWorkerJob(service);
	GThreadManager->Join();

}
