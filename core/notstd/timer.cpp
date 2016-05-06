#include "stdafx.h"
#include "timer.h"
#include "task.h"

///////////////////////////////////////////////////////////////////////////////

bool TimerHandleType::IsNullHandle(HandleType h)
{
	return (NULL == h);
}

void TimerHandleType::CloseHandleProc(HandleType h)
{
	::DeleteTimerQueueEx(h, INVALID_HANDLE_VALUE);
}

void TimerHandleType::SetHandleNull(HandleType &h)
{
	h = NULL;
}

///////////////////////////////////////////////////////////////////////////////

TimerSeed::TimerSeed()
	: mTimer(::CreateTimerQueue())
{
}

TimerSeed::~TimerSeed()
{
}

TimerSeed* TimerSeed::GetGlobalTimerSeed()
{
	return &theTimerSeed;
}

void CALLBACK TimerSeed::OnTimerCallback(PVOID parameter, BOOLEAN timerOrWaitFired)
{
	Task *task = reinterpret_cast<Task*>(parameter);
	if (task)
	{
		task->Run();
	}
}

HANDLE TimerSeed::CreateTimer(Task *task, 
	DWORD dueTime, DWORD perid, bool onlyOnce)
{
	HANDLE h = NULL;
	BOOL r = ::CreateTimerQueueTimer(&h, mTimer,
		reinterpret_cast<WAITORTIMERCALLBACK>(&TimerSeed::OnTimerCallback),
		task, dueTime, perid,
		WT_EXECUTEDEFAULT | (onlyOnce ? WT_EXECUTEONLYONCE : 0));
	return h;
}

void TimerSeed::DeleteTimer(HANDLE timer)
{
	::DeleteTimerQueueTimer(mTimer, timer, INVALID_HANDLE_VALUE);
}

///////////////////////////////////////////////////////////////////////////////

TimerSeed theTimerSeed;
