#pragma once

// C++11的variable_condition存在一些使用上的缺陷，所以还是提供
// 类似win32的event抽象来解决一些常见的线程同步问题

#include "config.h"
#if defined(PLATFORM_WINDOWS)
#include "event-win32.h"
#else
#include "event-posix.h"
#endif
