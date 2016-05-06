#pragma once

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

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlsave.h>

#pragma warning(push)
#pragma warning(disable: 4005)
#include <intsafe.h>
#include <stdint.h>
#pragma warning(pop)
#pragma warning(default: 4005)

///////////////////////////////////////////////////////////////////////////////

#if defined(PLATFORM_WINDOWS)
typedef void* HANDLE;
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef __AFX_H__
typedef void* POSITION;
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef PLATFORM_WINDOWS
# ifdef NOTSTD_EXPORTS
#  define NOTSTD_API __declspec(dllexport)
# else
#  define NOTSTD_API __declspec(dllimport)
# endif
#else
#define NOTSTD_API
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
