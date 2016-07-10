/*
	由于dataManager启动数据提供程序时，通过命令行参数给数据提供程序提供了很多
	信息。所以命令行的处理是所有数据提供程序都要涉及的。由于在本软件中，很多
	程序都设计为非mfc程序，所以，需要自行写一个命令行参数处理的模块。
*/

#pragma once
#include "config.h"
#include "simpleTool.h"

namespace notstd {

	class NOTSTD_API CommandlineParser {
	protected:
		virtual bool OnCommandlinePart(const std::string &param, bool isOption,
			const std::string &nextParam, bool &hasParam);

	public:
		virtual ~CommandlineParser();

		bool parseCommandLine(const std::string &commandLine);
		//bool parseCommandLine(int argc, char *argv[]);
	};

	///////////////////////////////////////////////////////////////////////////////

	struct NOTSTD_API ApplicationData {
		Handle<NormalHandleType> readOnlyHandle;
		int readOnlySize;

		Handle<NormalHandleType> readWriteHandle;
		int readWriteSize;

		Handle<NormalHandleType> stringHandle;
		int stringSize;

		std::string *namedPipeName;

		unsigned long parentProcessId;

		std::string *projRootPath;
		std::string *devTypeName;

		uint32_t theId;

		// for ps
		ULONG userData;

		ApplicationData();
		~ApplicationData();
	};

	class NOTSTD_API appCommandLine : public CommandlineParser {
		ApplicationData &mAppData;

	protected:
		virtual bool OnCommandlinePart(const std::string &param,
			bool isOption, const std::string &nextParam, bool &hasParam);

	public:
		appCommandLine(ApplicationData &appData);
	};

}
