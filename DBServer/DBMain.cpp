#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "DBPacketHandler.h"
#include "ThreadManager.h"
#include "Session.h"
#include "DBSession.h"

#include "Service.h"
#include "BufferWriter.h"

#include "Job.h"
#include "Logger.h"
#include "GLogger.h"
using namespace std;


void DoWorkerJob(DBServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		// IOCP 이벤트 처리
		service->GetIocpCore()->Dispatch(10);

		// 예약된 작업 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐 작업
		ThreadManager::DoGlobalQueueWork();

	}
}

int main()
{
	GLogger::Init("DBServer");
	// 패킷 핸들러 초기화
	DBPacketHandler::Init();

	// 서비스 시작
	DBServerServiceRef service = MakeShared<DBServerService>( // connect
		NetAddress(IP, DB_PORT),
		MakeShared<IocpCore>(),
		MakeShared<DBSession>,
		1
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

	// 메인 스레드도 워커로 사용
	DoWorkerJob(service);

	// 모든 스레드 종료 대기
	GThreadManager->Join();

	return 0;
}
