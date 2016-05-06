#pragma once

#ifdef CSCRIPTENG_EXPORTS
#define CSCRIPTENG_API __declspec(dllexport)
#else
#define CSCRIPTENG_API __declspec(dllimport)
#endif
