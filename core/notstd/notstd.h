#if defined(PLATFORM_WINDOWS)
#pragma once
#endif

#if !defined(_NOT_STD_H)
#define _NOT_STD_H

#include "config.h"
#include "vcverport.h"
#include <notstd/stringHelper.h>
#include <notstd/charconv.h>
#include <notstd/nslist.h>
#include <notstd/xmlparserHelper.h>
#include <notstd/md5.h>
#include <notstd/tuple.hpp>
#include <notstd/crc.h>
#include <notstd/objbase.h>
#include <notstd/filesystem.h>

#if defined(PLATFORM_WINDOWS)
#include <notstd/cmdline.h>
#include <notstd/simpleTool.h>
#include <notstd/pipe.h>
#include <notstd/timer.h>
#include <notstd/AsynTask.h>
#include <notstd/task.h>
#include <notstd/sockmgr.h>
#include <notstd/simpleThread.h>
#endif

#endif
