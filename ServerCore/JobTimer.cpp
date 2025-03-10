#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"
#include "Logger.h"

/*---------------
	JobTimer
-----------------*/

void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job)
{
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	WRITE_LOCK;
	_items.push(TimerItem{ executeTick, jobData}); // 쓰레드세이프 X

}

void JobTimer::Distribute(uint64 now)
{
	// 한번에 한 쓰레드만 통과
	if (_distributing.exchange(true) == true)
		return;
	
	Vector<TimerItem> items;
	//LOG_PUSH("JobTimer Distribute Start");
	{
		WRITE_LOCK;

		while (_items.empty() == false)
		{
			const TimerItem& timerItem = _items.top();
			if (now < timerItem.executeTick)
				break;

			items.push_back(timerItem);
			_items.pop();
		}
	}

	for (TimerItem& item : items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock())
			owner->Push(item.jobData->job);

		ObjectPool<JobData>::Push(item.jobData);

	}
	// 끝났으면 풀기
	//LOG_PUSH("JobTimer Distribute End");
	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;
	while (_items.empty() == false)
	{
		const TimerItem& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem.jobData);
		_items.pop();
	}
}
