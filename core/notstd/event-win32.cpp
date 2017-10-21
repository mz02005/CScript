#include "stdafx.h"
#include "event-win32.h"

namespace notstd {
	Event::Event()
		: mEventHandle(nullptr)
	{
	}

	Event::~Event()
	{
		CloseEvent();
	}

	bool Event::CreateEvent(bool manualReset, bool initState)
	{
		mEventHandle = ::CreateEvent(nullptr, !!manualReset, !!initState, nullptr);
		return mEventHandle != nullptr;
	}

	void Event::CloseEvent()
	{
		if (mEventHandle)
		{
			::CloseHandle(mEventHandle);
			mEventHandle = nullptr;
		}
	}

	bool Event::SetEvent()
	{
		return !!::SetEvent(mEventHandle);
	}

	bool Event::ResetEvent()
	{
		return !!::ResetEvent(mEventHandle);
	}

	bool Event::WaitObject(uint32_t timeout)
	{
		DWORD waitTime = timeout == (uint32_t)-1 ? INFINITE : timeout;
		DWORD r = ::WaitForSingleObject(mEventHandle, waitTime);
		return r == WAIT_OBJECT_0;
	}
}
