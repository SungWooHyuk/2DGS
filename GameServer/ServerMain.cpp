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
#include <spdlog/spdlog.h>

#include "MapData.h"
#include "DBGameSession.h"

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
void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		// ��Ʈ��ũ ����� ó�� -> �ΰ��� �������� (��Ŷ �ڵ鷯�� ����)
		service->GetIocpCore()->Dispatch(10);

		// ����� �ϰ� ó��
		ThreadManager::DistributeReservedJobs();

		// �۷ι� ť
		ThreadManager::DoGlobalQueueWork();

		//ThreadManager::DoLogger();
	}
}

int main()
{
	spdlog::info("Hello, spdlog!");
	//MAPDATA.InitMAP();
	//ClientPacketHandler::Init();
	//GameDBPacketHandler::Init();
	//GAMESESSIONMANAGER.InitializeNPC();
	////Logger::GetInstance().Init("GameServer");

	//ServerServiceRef service = MakeShared<ServerService>(
	//	NetAddress(L"127.0.0.1", 4000),
	//	MakeShared<IocpCore>(),
	//	MakeShared<GameSession>,
	//	MAX_USER
	//	);

	//ServerServiceRef dbservice = MakeShared<ServerService>(
	//	NetAddress(L"127.0.0.1", DB_PORT),
	//	MakeShared<IocpCore>(),
	//	MakeShared<DBGameSession>,
	//	1
	//	);


	//ASSERT_CRASH(service->Start());
	//ASSERT_CRASH(dbservice->Start());

	//int num_threads = std::thread::hardware_concurrency() - 3;
	//int db_num_threads = 3;

	//for (int32 i = 0; i < num_threads; ++i)
	//{
	//	GThreadManager->Launch([&service]()
	//		{
	//			DoWorkerJob(service);
	//		});
	//}

	//for (int32 i = 0; i < db_num_threads; ++i)
	//{
	//	GThreadManager->Launch([&dbservice]()
	//		{
	//			DoWorkerJob(dbservice);
	//		});
	//}

	//DoWorkerJob(service);
	//GThreadManager->Join();

}
