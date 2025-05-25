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
#include "DropTable.h"
#include "RedisManager.h"
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
	GLogger::Initialize("GameServer"); // log init
	GLogger::SetCurrentLogger("GameServer");
	MAPDATA.InitMAP();
	ClientPacketHandler::Init();
	GameDBPacketHandler::Init();
	GAMESESSIONMANAGER->InitializeNPC();
	REDIS.Init();
	ITEM.LoadFromJson("items.json");
	GDROPTABLE.LoadFromJson("droptable.json");
	GLogger::Log(spdlog::level::info, "Server Begin");

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

	int num_threads = std::thread::hardware_concurrency();
	int db_num_threads = num_threads/5;

	for (int32 i = 0; i < num_threads; ++i)
	{
		GThreadManager->Launch([&service]()
			{
				DoGameWorkerJob(service);
			});
	}

	for (int32 i = 0; i < db_num_threads; ++i)
	{
		GThreadManager->Launch([&dbservice]()
			{
				DoDBWorkerJob(dbservice);
			});
	}


	DoGameWorkerJob(service);
	GThreadManager->Join();
	spdlog::shutdown();
}
