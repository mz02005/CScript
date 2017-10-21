#pragma once
#include "eventbase.h"

namespace notstd {
	class NOTSTD_API Event : public WaitableObject
	{
		HANDLE mEventHandle;

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
