#pragma once

#include "config.h"
#if defined(PLATFORM_WINDOWS)
#include "ioServer-win32.h"
#else
#include "ioServer-posix.h"
#endif
