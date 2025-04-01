#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Job.h"
#include "Logger.h"

#include "DataBase.h"
#include "MapData.h"

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

using namespace std;


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
	DB->InitDB();
	MAPDATA->InitMAP();
	ClientPacketHandler::Init();
	GAMESESSIONMANAGER->InitializeNPC();
	//Logger::GetInstance().Init("GameServer");

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 4000),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		MAX_USER
		);

	ASSERT_CRASH(service->Start());
	int num_threads = std::thread::hardware_concurrency();

	for (int32 i = 0; i < num_threads; ++i)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}

	DoWorkerJob(service);
	GThreadManager->Join();

}
