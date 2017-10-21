#include "stdafx.h"
#include "event-posix.h"

#if !defined(PLATFORM_WINDOWS)

namespace notstd {
	Event::Event()
		: mIsCondCreate(false)
		, mIsMutexCreate(false)
		, mIsManualReset(false)
		, mIsSignalStatus(false)
	{
	}

	Event::~Event()
	{
		CloseEvent();
	}

	bool Event::CreateEvent(bool manualReset, bool initState)
	{
		int r = 0;
		if (!mIsCondCreate)
		{
			r = pthread_cond_init(&mCond, nullptr);
			if (!r)
				mIsCondCreate = true;
			else
				return false;
		}

		if (!mIsMutexCreate)
		{
			r = pthread_mutex_init(&mMutex, nullptr);
			if (!r)
				mIsMutexCreate = true;
			else
			{
				mIsCondCreate = false;
				pthread_cond_destroy(&mCond);
				return false;
			}
		}
		return r >= 0;
	}

	void Event::CloseEvent()
	{
		if (mIsMutexCreate)
		{
			mIsMutexCreate = false;
			pthread_mutex_destroy(&mMutex);
		}
		if (mIsCondCreate)
		{
			mIsCondCreate = false;
			pthread_cond_destroy(&mCond);
		}
	}

	bool Event::SetEvent()
	{
		if (!isInitialized())
			return false;

		pthread_mutex_lock(&mMutex);
		mIsSignalStatus = true;
		pthread_cond_broadcast(&mCond);
		pthread_mutex_unlock(&mMutex);

		return true;
	}

	bool Event::ResetEvent()
	{
		if (!isInitialized())
			return false;
		pthread_mutex_lock(&mMutex);
		mIsSignalStatus = false;
		pthread_mutex_unlock(&mMutex);
		return true;
	}

	bool Event::WaitObject(uint32_t timeout)
	{
		if (!isInitialized())
			return false;

		int ret;
		pthread_mutex_lock(&mMutex);
		if (timeout != -1)
		{
			struct timeval tv;
			gettimeofday(&tv, nullptr);

			struct timespec ts;
			ts.tv_sec = tv.tv_sec + (timeout / 1000);
			ts.tv_nsec = tv.tv_usec * 1000 + (timeout % 1000) * 1000000;

			if (ts.tv_nsec >= 1000000000)
			{
				ts.tv_sec++;
				ts.tv_nsec -= 1000000000;
			}

			ret = pthread_cond_timedwait(&mCond, &mMutex, &ts);
		}
		else
		{
			ret = 0;
			while (!mIsSignalStatus && !ret)
				ret = pthread_cond_wait(&mCond, &mMutex);
		}

		// 处理自动恢复状态
		if (!ret && !mIsManualReset)
			mIsSignalStatus = false;

		pthread_mutex_unlock(&mMutex);

		return true;
	}
}

#endif
