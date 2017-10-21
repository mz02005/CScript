#include "stdafx.h"
#include "sockbase.h"

#if !defined(PLATFORM_WINDOWS)

namespace notstd {
	
	bool NetEnviroment::GetExtFuncPointer()
	{
		return true;
	}

	bool NetEnviroment::Init()
	{
		return true;
	}

	void NetEnviroment::Term()
	{
	}
}

#endif
