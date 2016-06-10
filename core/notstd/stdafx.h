#pragma once
#include "notstd/config.h"

#ifdef PLATFORM_WINDOWS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#endif
