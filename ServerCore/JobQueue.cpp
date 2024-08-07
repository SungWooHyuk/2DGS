#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"

/*-----------------
	JobQueue
------------------*/

void JobQueue::Push(JobRef job, bool pushOnly)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	// 첫번째 job을 넣은 thread. 실행까지 담당
	if (prevCount == 0)
	{

		//이미 실행중인 JobQueue가 없으면 실행
		if (LCurrentJobQueue == nullptr && pushOnly == false)
		{
			Execute();
		}
		else
		{
			// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다.
			GGlobalQueue->Push(shared_from_this());
		}
	}
}

// 1) 일감이 너무 몰린다면?
// 2) DoAsync 타고 타고 가서 절대 끝나지않는상황 - 일감이 한 쓰레드에 몰릴때.

void JobQueue::Execute()
{
	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; ++i)
			jobs[i]->Execute();

		// 원래 값이 내가 빼준 값이랑 동일했다고 하면 결과물 자체가 0이라는얘기
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			// 남은 일감이 0개라면 종료
			LCurrentJobQueue = nullptr;
			return;
		}

		const uint64 now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;
			// 여유 있는 다른 쓰레드가 실행하도록 Globalqueue에 넘김
			GGlobalQueue->Push(shared_from_this());
			break;
		}
	}
}

