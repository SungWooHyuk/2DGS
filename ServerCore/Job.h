#pragma once
#include <functional>

/*---------
	Job
----------*/

using CallbackType = std::function<void()>;

class Job
{
public:
	Job(CallbackType&& callback) : _callback(std::move(callback))
	{
	}

	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		_callback = [owner, memFunc, args...]()
		{
			(owner.get()->*memFunc)(args...);
		};
	}
	// 캡쳐를 하건 펑터를 만들건 어떤 아니면 그냥 클래스에서 들고 있건 shared_ptr을 
	// 들고있는순간 생명유지에는 도움을 주기는 하지만 사이클이 일어나지않게(메모리릭)이 조심해야한다.
	
	void Execute()
	{
		_callback();
	}

private:
	CallbackType _callback;
};

