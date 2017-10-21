#pragma once
#include "config.h"

#define SCRIPT_TRACE scriptLog::LogTool()
#define SCRIPT_TRACE_(flags) scriptLog::LogTool((uint32_t)flags)

namespace scriptLog {
	class LogTool
	{
	public:
		enum {
			TraceGeneral = 0x000001,
			TraceCom = 0x000002,
			TraceQI = 0x000004,
			TraceRegistrar = 0x000008,
			TraceRefcount = 0x000010,
			TraceWindowing = 0x000020,
			TraceControls = 0x000040,
			TraceHosting = 0x000080,
			TraceDBClient = 0x000100,
			TraceDBProvider = 0x000200,
			TraceSnapin = 0x000400,
			TraceNotImpl = 0x000800,
			TraceAllocation = 0x001000,
			TraceException = 0x002000,
			TraceTime = 0x004000,
			TraceCache = 0x008000,
			TraceStencil = 0x010000,
			TraceString = 0x020000,
			TraceMap = 0x040000,
			TraceUtil = 0x080000,
			TraceSecurity = 0x100000,
			TraceSync = 0x200000,
			TraceISAPI = 0x400000,
			TraceUser = 0x80000,

			TraceAll = (uint32_t)-1,
		};

	private:
		// 需要显示哪些类型的信息
		static uint32_t mLogFlags;

		uint32_t mMyFlags;
		bool mToTrace;

	public:
		static void SetLogFlags(uint32_t newFlags);
		static uint32_t GetLogFlags() { return mLogFlags; }
		LogTool(uint32_t theFlags = 0);
		void operator()(const char *format, ...);
	};
}
