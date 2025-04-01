#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferWriter.h"
#include "Job.h"
#include "Logger.h"
#include <concurrent_priority_queue.h>
#include <sqlext.h>
#include <locale>

using namespace std;


void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러에 의해)
		service->GetIocpCore()->Dispatch(10);

		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐
		ThreadManager::DoGlobalQueueWork();

		//ThreadManager::DoLogger();
	}
}

int main()
{
	DB->InitDB();
	//ClientPacketHandler::Init();
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
