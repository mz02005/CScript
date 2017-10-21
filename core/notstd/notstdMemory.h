#pragma once
#include "config.h"

namespace notstd {

	NOTSTD_API void* MemAllocate(size_t size);
	NOTSTD_API void MemFree(void *p);
	NOTSTD_API void* MemReallocate(void* p, size_t size);

}
