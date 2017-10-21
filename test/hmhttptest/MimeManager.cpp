#include "stdafx.h"
#include "hmhttptest.h"
#include "notstd/notstd.h"
#include <fstream>

int MimeManager::LoadMimeConf()
{
	std::string path = notstd::AppHelper::GetAppDir();
	path += "mime.types";

	return LoadMimeConfInner(path);
}

int MimeManager::LoadMimeConfInner(const std::string &filePathName)
{
	std::string str;

	std::fstream f(filePathName.c_str(), std::ios::in);
	while (std::getline(f, str))
	{
		if (str.find('{') != str.npos
			|| str.find('}') != str.npos)
			continue;

		notstd::StringHelper::Replace(str, "\r", "");
		notstd::StringHelper::Replace(str, "\n", "");
		notstd::StringHelper::Replace(str, ";", "");
		notstd::StringHelper::Trim(str, " \t");

		auto ss = notstd::StringHelper::SplitString(str, " \t");
		if (ss.size() > 1)
		{
			for (size_t x = 1; x < ss.size(); x++)
			{
				mMimeTypeTable.insert(std::make_pair(ss[x], ss[0]));
			}
		}
	}

	return 0;
}

std::string MimeManager::GetMimeType(const std::string &extName) const
{
	auto iter = mMimeTypeTable.find(extName);
	if (iter != mMimeTypeTable.end())
		return iter->second;
	return "";
}
