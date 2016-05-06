#include "stdafx.h"
#include "cmdline.h"
#include "stringHelper.h"

CommandlineParser::~CommandlineParser()
{
}

bool CommandlineParser::OnCommandlinePart(const std::string &param, 
	bool isOption, const std::string &nextParam, bool &hasParam)
{
	return false;
}

//bool CommandlineParser::parseCommandLine(int argc, char *argv[])
//{
//	for (int i = 1; i < argc; i ++)
//	{
//		bool hasParam = false;
//		char c = argv[i][0];
//		if (c == '/' || c == '-')
//		{
//			if (!OnCommandlinePart(&argv[i][1], true, i < argc - 1 ? argv[i + 1] : "", hasParam))
//				return false;
//			if (hasParam)
//				i ++;
//		}
//		else
//		{
//			if (!OnCommandlinePart(argv[i], false, "", hasParam))
//				return false;
//		}
//	}
//	return true;
//}

bool CommandlineParser::parseCommandLine(const std::string &cmdLine)
{
	bool inQuotation = false;
	std::string s;
	std::vector<std::string> paramList;
	const char *p = cmdLine.c_str(), *e = cmdLine.c_str() + cmdLine.size();
	while (p != e)
	{
		char c = *p++;
		if (c == '\t' || c == ' ')
		{
			if (inQuotation)
				s += c;
			else
			{
				if (!s.empty()) {
					paramList.push_back(s);
					s.clear();
				}
			}
		}
		else if (c == '\"')
		{
			if (!inQuotation)
				inQuotation = true;
			else
			{
				inQuotation = false;
			}
		}
		else
			s += c;
	}
	if (!s.empty())
		paramList.push_back(s);

	if (inQuotation)
		return false;

	auto theParamSize = paramList.size();
	for (decltype(theParamSize) i = 0; i < theParamSize; i++)
	{
		const std::string &v = paramList[i];
		bool isFlag = v[0] == '/';
		bool hasParam = false;
		if (!OnCommandlinePart(v.c_str() + (isFlag ? 1 : 0), isFlag, i < theParamSize - 1 ? paramList[i + 1] : "", hasParam))
			return false;
		if (hasParam)
			++i;
	}

	return true;
}

//bool CommandlineParser::parseCommandLine(const std::string &commandLine)
//{
//	StringHelper::StringArray sa;
//	sa = StringHelper::SplitString(commandLine, "\t ");
//
//	int countAfterCombine = 0;
//	int totalPartCount = sa.size();
//	std::vector<std::string> argvColl(totalPartCount);
//	std::vector<char*> argv(totalPartCount);
//	for (int i = 0; i < totalPartCount; i ++)
//	{
//		if (sa[i][0] == '\"')
//		{
//			argvColl[countAfterCombine].append(&sa[i][1]);
//			while (i < totalPartCount - 1 && sa[++i].back() != '\"')
//			{
//				argvColl[countAfterCombine].append(sa[i]);
//			}
//			if (i >= totalPartCount)
//			{
//				// 没有用引号结束
//				return false;
//			}
//			argvColl[countAfterCombine++].append(sa[i].begin(), sa[i].end() - 1);
//		}
//		else
//		{
//			argvColl[countAfterCombine++].append(sa[i]);
//		}
//	}
//	for (int i = 0; i < countAfterCombine; i ++)
//	{
//		argv[i] = &argvColl[i][0];
//	}
//	return parseCommandLine(countAfterCombine, &argv[0]);
//}

///////////////////////////////////////////////////////////////////////////////

ApplicationData::ApplicationData()
	: readOnlySize(-1)
	, readWriteSize(-1)
	, stringSize(-1)
	, namedPipeName(new std::string)
	, devTypeName(new std::string)
	, projRootPath(new std::string)
	, parentProcessId(0)
	, theId(0)
	, userData(0)
{
}

ApplicationData::~ApplicationData()
{
	delete projRootPath;
	delete devTypeName;
	delete namedPipeName;
}

///////////////////////////////////////////////////////////////////////////////

bool appCommandLine::OnCommandlinePart(const std::string &param, 
	bool isOption, const std::string &nextParam, bool &hasParam)
{
	if (!isOption) {
		*mAppData.devTypeName = param;
		hasParam = false;
		return true;
	}

	if (param.size() >= 4 && param.substr(0, 4) == "proj")
	{
		printf("Project root is [%s].\n", param.substr(4).c_str());
		*mAppData.projRootPath = param.substr(4).c_str();
	}
	else if (param.size() >= 4 && param.substr(0, 4) == "psud")
	{
		//mAppData.userData = static_cast<ULONG>(strtoull(param.substr(4).c_str(), NULL, 10));
	}
	else if (param.size() >= 3 && param.substr(0, 3) == "ros")
	{
		mAppData.readOnlySize = atoi(param.substr(3).c_str());
	}
	else if (param.size() >= 2 && param.substr(0, 2) == "ro")
	{
		int readOnlyHandleV = atoi(param.substr(2).c_str());
		mAppData.readOnlyHandle = *reinterpret_cast<HANDLE*>(&readOnlyHandleV);
	}
	else if (param.size() >= 3 && param.substr(0, 3) == "rws")
	{
		mAppData.readWriteSize = atoi(param.substr(3).c_str());
	}
	else if (param.size() >= 2 && param.substr(0, 2) == "rw")
	{
		int readWriteHandleV = atoi(param.substr(2).c_str());
		mAppData.readWriteHandle = *reinterpret_cast<HANDLE*>(&readWriteHandleV);
	}
	else if (param.size() >= 4 && param.substr(0, 4) == "strs")
	{
		mAppData.stringSize = atoi(param.substr(4).c_str());
	}
	else if (param.size() >= 3 && param.substr(0, 3) == "str")
	{
		int stringHandleV = atoi(param.substr(3).c_str());
		mAppData.stringHandle = *reinterpret_cast<HANDLE*>(&stringHandleV);
	}
	else if (param.size() >= 2 && param.substr(0, 2) == "pn")
	{
		*mAppData.namedPipeName = param.substr(2);
	}
	else if (param.size() >= 3 && param.substr(0, 3) == "ppi")
	{
		int parentProcessIdV = atoi(param.substr(3).c_str());
		mAppData.parentProcessId = *reinterpret_cast<unsigned long*>(&parentProcessIdV);
	}
	// theId
	else if (param.size() >= 5 && param.substr(0, 5) == "theId")
	{
		mAppData.theId = strtoul(param.substr(5).c_str(), NULL, 10);
	}
	else
	{
		return CommandlineParser::OnCommandlinePart(param, isOption, nextParam, hasParam);
	}
	return true;
}

appCommandLine::appCommandLine(ApplicationData &appData)
	: mAppData(appData)
{
}
