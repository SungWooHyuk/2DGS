#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "pch.h"

#include <thread>
#include <mutex>
#include <atomic>

#include "ThreadManager.h"
#include "Service.h"

#include "DummySession.h"
#include "DummyManager.h"
#include "Client.h"
#include "TestPacketHandler.h"

extern HWND		hWnd;

vector <thread*> worker_threads;
thread test_thread;

void Test_Thread()
{
	while (true) {
		if (GDummyManager->Adjust_Number_Of_Client())
			GDummyManager->AllMove();
	}
}

void Worker_Thread(DummyServiceRef service)
{
	while (true)
	{
		service->GetIocpCore()->Dispatch(10);
	}
}
void InitializeNetwork()
{
	TestPacketHandler::Init();

	DummyServiceRef service = MakeShared<DummyService>(
		NetAddress(L"127.0.0.1", PORT_NUM),
		MakeShared<IocpCore>(),
		MakeShared<DummySession>,
		1
		);

	ASSERT_CRASH(service->Start());
	GDummyManager->SetService(service);
	int num_threads = std::thread::hardware_concurrency();

	for (int32 i= 0; i < 6; ++i)
	{
		worker_threads.push_back(new std::thread([service]() {
			Worker_Thread(service);
			}));
	}

	test_thread = thread{ Test_Thread };
}

void ShutdownNetwork()
{
	test_thread.join();
	for (auto pth : worker_threads) {
		pth->join();
		delete pth;
	}
}
