#pragma once
#include "config.h"
#include "contrib/minizip/unzip.h"
#include "contrib/minizip/zip.h"
#include <string>

namespace notstd {
	class NOTSTD_API zlibHelper
	{
	public:
		enum 
		{
			E_SUCCEEDED,
			E_UNKNOWNERROR,
			E_NOTIMPLEMENT,
			E_PARAMERROR,
			E_OPENFAIL,
			E_GETFILEINFOFAIL,
			E_CREATEFILEFAIL,
		};

		static const char mSplash[2];

	private:
		static bool IsDirectoryExists(const std::string &dir);
		static int CreateDirectoryInner(const std::string &dir);
		static bool AddFile2Zip(const std::string &filePath,
			const std::string &relpath, zipFile zipfile);

	public:
		static int CreateDirectoryR(const std::string &dir);

		struct UnzipParam
		{
			int showVerb;
		};
		// 将zip文件解压缩到目录destDir中
		static int unzipToDirectory(const std::string &destDir, const std::string &srcFile, 
			const UnzipParam *param = nullptr);

		// 将一个目录下的所有文件压缩到指定的zip文件中
		static int zipDirectoryToFile(const std::string &destFile, const std::string &srcDirectory);
	};
}
