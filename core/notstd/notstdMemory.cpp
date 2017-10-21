#include "stdafx.h"
#include "notstdMemory.h"

namespace notstd {

	NOTSTD_API void* MemAllocate(size_t size)
	{
		return malloc(size);
	}

	NOTSTD_API void MemFree(void *p)
	{
		free(p);
	}

	NOTSTD_API void* MemReallocate(void* p, size_t size)
	{
		return realloc(p, size);
	}

}
