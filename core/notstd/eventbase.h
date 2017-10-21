#pragma once
#include "config.h"

namespace notstd {
	class NOTSTD_API WaitableObject
	{
	public:
		virtual ~WaitableObject();
		virtual bool WaitObject(uint32_t timeout = -1);
	};
}
