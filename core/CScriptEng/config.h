#pragma once

#include "notstd/config.h"

#if defined(PLATFORM_WINDOWS)
#ifdef CSCRIPTENG_EXPORTS
#define CSCRIPTENG_API __declspec(dllexport)
#else
#define CSCRIPTENG_API __declspec(dllimport)
#endif
#else
#define CSCRIPTENG_API
#endif
