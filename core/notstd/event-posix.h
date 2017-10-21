#pragma once
#include "config.h"

#if !defined(PLATFORM_WINDOWS)
#include <sys/time.h>
#include <pthread.h>
#include "eventbase.h"

namespace notstd {
	class NOTSTD_API Event : public WaitableObject
	{
	private:
		bool mIsCondCreate;
		bool mIsMutexCreate;
		bool mIsManualReset;
		bool mIsSignalStatus;

		// 标准库里的mutex还是不错的
		pthread_mutex_t mMutex;

		// 标准库里的variable_condition不太理想，还是用linux系统自带的
		pthread_cond_t mCond;

		inline bool isInitialized() const { return mIsCondCreate && mIsMutexCreate; }

	public:
		Event();
		virtual ~Event();

		bool CreateEvent(bool manualReset, bool initState = false);
		void CloseEvent();

		bool SetEvent();
		bool ResetEvent();

		virtual bool WaitObject(uint32_t timeout = 0) override;
	};
}

#endif
