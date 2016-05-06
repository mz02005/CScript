#include "stdafx.h"
#include "CScriptEng.h"

using namespace scriptLog;

///////////////////////////////////////////////////////////////////////////////

uint32_t LogTool::mLogFlags = LogTool::TraceAll;

LogTool::LogTool(uint32_t theFlags)
	: mMyFlags(theFlags)
	, mToTrace((mMyFlags & mLogFlags) == mMyFlags)
{
}

void LogTool::operator()(const char *format, ...)
{
	if (!mToTrace)
		return;

	std::string s;
	va_list valist;
	va_start(valist, format);
	StringHelper::FormatV(s, format, valist);
	va_end(valist);

	OutputDebugStringA(s.c_str());
	printf(s.c_str());
}

///////////////////////////////////////////////////////////////////////////////
