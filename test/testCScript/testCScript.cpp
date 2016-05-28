#include "stdafx.h"
#include "CScriptEng/CScriptEng.h"
#include "CScriptEng/vm.h"
#include <iostream>

int ExecuteCode(const std::wstring &filePathName, bool saveCodeToFile = false);

void RunTestCase();

int _tmain(int argc, _TCHAR* argv[])
{
	scriptAPI::SimpleCScriptEng::Init();
	if (argc == 1)
	{
		RunTestCase();
	}
	else
	{
		ExecuteCode(argv[1], true);
	}
	scriptAPI::SimpleCScriptEng::Term();
	return 0;
}

void RunTestCase()
{
	std::wstring appDir;
	appDir.resize(MAX_PATH);
	appDir.resize(::GetModuleFileNameW(nullptr, &appDir[0], MAX_PATH));
	appDir.resize(appDir.rfind(L'\\') + 1);

	appDir += L"..\\test\\script\\";

	std::list<std::wstring> fileDirList;
	fileDirList.push_back(appDir);

	while (!fileDirList.empty())
	{
		std::wstring curDir = fileDirList.front();
		fileDirList.pop_front();
		std::wstring toFind = curDir + L"*.c";

		HANDLE f;
		WIN32_FIND_DATAW fd;
		if ((f = ::FindFirstFile(toFind.c_str(), &fd)) != INVALID_HANDLE_VALUE)
		{
			do {
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (fd.cFileName[0] != L'.' && wcscmp(fd.cFileName, L".."))
						fileDirList.push_back(curDir + fd.cFileName + L"\\");
				}
				else
				{
					ExecuteCode(curDir + fd.cFileName);
				}
			} while (::FindNextFileW(f, &fd));
			::FindClose(f);
		}
	}
}

int ExecuteCode(const std::wstring &filePathName, bool saveCodeToFile)
{
	std::wcout << L"Source file: " << filePathName << std::endl;

	scriptAPI::FileStream fs(notstd::ICONVext::unicodeToMbcs(filePathName).c_str());
	scriptAPI::ScriptCompiler compiler;

	std::wcout << L"Compile file " << filePathName << std::endl;
	HANDLE h = compiler.Compile(&fs, true);
	if (h)
	{
		if (saveCodeToFile)
		{
			// 保存到和被执行的代码相同的路径中，扩展名为.txt
			std::wstring codePath = filePathName + L".txt";
			FILE *file = nullptr;
			::_wfopen_s(&file, codePath.c_str(), L"wb");
			do {
				if (!file)
				{
					printf("Open code file file.\n");
					break;
				}
				if (
					compiler.SaveCodeToFile(h, file) < 0
					|| compiler.SaveConstStringTableInResultToFile(h, file) < 0)
				{
					printf("Save code fail.\n");
				}
				::fclose(file);
			} while (0);
		}

		std::wcout << L"Compile file success. Start to execute. " << std::endl;
		scriptAPI::ScriptRuntimeContext *runtimeContext
			= scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext(512, 512);
		runtimeContext->Execute(h);
		scriptAPI::ScriptCompiler::ReleaseCompileResult(h);
		scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(runtimeContext);
		return 0;
	}
	else
		std::wcout << L"Compile file " << filePathName << L" fail. \n\n" << std::endl;

	return -1;
}
