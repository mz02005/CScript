#pragma once
#include <cstddef>

#if defined(WIN32) || defined(_WINDOWS) || defined(_WINNT_) || defined(_WINDOWS_)
#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS
#endif
#endif

#ifdef PLATFORM_WINDOWS
#pragma once
#endif

#ifndef NOTSTD_CONFIG_H
#define NOTSTD_CONFIG_H

#ifdef PLATFORM_WINDOWS
#pragma warning(disable: 4819)
#pragma warning(disable: 4996)
#endif

#if defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif
#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#ifdef PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable: 4005)
#include <intsafe.h>
#include <stdint.h>
#pragma warning(pop)
#pragma warning(default: 4005)
#else
#include <stdint.h>
#endif

///////////////////////////////////////////////////////////////////////////////

#if !defined(PLATFORM_WINDOWS)
typedef void* HANDLE;
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef __AFX_H__
typedef void* POSITION;
#endif

///////////////////////////////////////////////////////////////////////////////

#if !defined(PLATFORM_WINDOWS)
#if !defined(MAX_PATH)
#define MAX_PATH 260
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef PLATFORM_WINDOWS
#ifdef _STATIC_LIB
#define NOTSTD_API
#else
# ifdef NOTSTD_EXPORTS
#  define NOTSTD_API __declspec(dllexport)
# else
#  define NOTSTD_API __declspec(dllimport)
# endif
#endif
#else
#define NOTSTD_API
#endif

///////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <string.h>
#if defined(_DEBUG) || defined(DEBUG)
#define NOTSTD_ASSERT assert
#else
#define NOTSTD_ASSERT
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef PLATFORM_WINDOWS
#define STDCALL_API __stdcall
#else
#define STDCALL_API
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
