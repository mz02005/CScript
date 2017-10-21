#include "stdafx.h"
#include "simpleThread.h"
#include <process.h>

#if defined(PLATFORM_WINDOWS)

namespace notstd {
	SimpleThread::SimpleThread()
	{
	}

	SimpleThread::~SimpleThread()
	{
		stopThread();
	}

	bool SimpleThread::shouldIStop(unsigned long timeout)
	{
		DWORD r = ::WaitForSingleObject(mStopEvent, timeout);
		if (r != WAIT_TIMEOUT)
			return true;
		return false;
	}

	bool SimpleThread::isStopped() {
		DWORD r = ::WaitForSingleObject(mThread, 0);
		if (r != WAIT_TIMEOUT)
			return true;
		return false;
	}

	UINT WINAPI SimpleThread::ThreadProcInner(LPVOID lParam)
	{
		SimpleThread *thread = reinterpret_cast<SimpleThread*>(lParam);
		if (thread && thread->mOnThread)
			(*thread->mOnThread)(lParam);
		return 0;
	}

	bool SimpleThread::startThread(OnThreadProc onThread, void *lParam)
	{
		stopThread();

		mUser = lParam;

		mStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!mStopEvent)
			return false;

		mOnThread = onThread;
		mThread = (HANDLE)_beginthreadex(NULL, 0, &SimpleThread::ThreadProcInner, this, 0, NULL);
		bool r = !!mThread;
		if (!r)
			mThread.Close();

		return r;
	}

	void SimpleThread::stopThread()
	{
		::SetEvent(mStopEvent);
		::WaitForSingleObject(mThread, INFINITE);
		mStopEvent.Close();
		mThread.Close();
	}

	///////////////////////////////////////////////////////////////////////////////

	TaskThread::TaskThread()
	{
	}

	TaskThread::~TaskThread()
	{
		stopTaskThread();
	}

	bool TaskThread::startTaskThread()
	{
		return mThread.startThread(&TaskThread::OnTaskThread, this);
	}

	void TaskThread::stopTaskThread()
	{
		mThread.stopThread();
		mTaskManager.DoTask();
	}

	void TaskThread::OnTaskThread(void *thread)
	{
		SimpleThread *simpleThread = reinterpret_cast<SimpleThread*>(thread);
		TaskThread *taskThread = reinterpret_cast<TaskThread*>(simpleThread->GetUserData());
		taskThread->TaskThreadProc();
	}

	void TaskThread::TaskThreadProc()
	{
		while (!mThread.shouldIStop(50))
		{
			OnIdle(notstd::GetCurrentTick());
			mTaskManager.DoTask();
		}
	}

	void TaskThread::OnIdle(DWORD tick)
	{
	}
}

#endif
