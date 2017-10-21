#pragma once
#include "config.h"
#include "simpleTool.h"

namespace notstd {

	class Task;

	class NOTSTD_API TimerHandleType
	{
	public:
		typedef HANDLE HandleType;

		static bool IsNullHandle(HandleType h);
		static void CloseHandleProc(HandleType h);
		static void SetHandleNull(HandleType &h);
	};

	template class NOTSTD_API Handle < TimerHandleType >;

	class NOTSTD_API TimerSeed
	{
	protected:
		Handle<TimerHandleType> mTimer;

	protected:
		static void CALLBACK OnTimerCallback(PVOID parameter, BOOLEAN timerOrWaitFired);

	public:
		TimerSeed();
		virtual ~TimerSeed();

		operator HANDLE() {
			return mTimer;
		}

		static TimerSeed* GetGlobalTimerSeed();

		HANDLE CreateTimer(Task *task, DWORD dueTime, DWORD perid, bool onlyOnce = false);
		void DeleteTimer(HANDLE timer);
	};

	extern TimerSeed theTimerSeed;
}
