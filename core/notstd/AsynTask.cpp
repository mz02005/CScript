#include "stdafx.h"
#include "AsynTask.h"

namespace notstd {
	AsyncTaskManager::~AsyncTaskManager()
	{
		Release();
	}

	void AsyncTaskManager::OnDo(Task *task)
	{
		task->Run();
	}

	void AsyncTaskManager::OnRelease(Task *task)
	{
		task->Release();
	}

	void AsyncTaskManager::PostTask(Task *task)
	{
		AddItem(task);
	}

	void AsyncTaskManager::DoTask(long long timeout)
	{
		JustDoIt(timeout);
	}
}
