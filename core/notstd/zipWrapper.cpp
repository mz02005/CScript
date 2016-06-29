#include "stdafx.h"
#include "zipWrapper.h"
#include "notstd/notstd.h"

#if defined(PLATFORM_WINDOWS)
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace notstd {
#if defined(PLATFORM_WINDOWS)
	const char zlibHelper::mSplash[2] = { '\\', 0 };
#else
	const char zlibHelper::mSplash[2] = { '/', 0 };
#endif

	// 创建一级目录
	int zlibHelper::CreateDirectoryInner(const std::string &dir)
	{
#if defined(PLATFORM_WINDOWS)
		if (!::CreateDirectoryA(dir.c_str(), nullptr))
			return ::GetLastError() == ERROR_ALREADY_EXISTS;
		return -E_UNKNOWNERROR;
#else
		return mkdir(dir.c_str(), 0777);
#endif
	}

	bool zlibHelper::IsDirectoryExists(const std::string &dir)
	{
#if defined(PLATFORM_WINDOWS)
		return !!::PathIsDirectoryA(dir.c_str());
#else
		DIR *thedir = opendir(dir.c_str());
		if (!thedir)
			return false;
		closedir(thedir);
		return true;
#endif
	}

	int zlibHelper::CreateDirectoryR(const std::string &dir)
	{
		if (dir.empty())
			return -E_PARAMERROR;

		std::string path = dir;
		if (*path.rbegin() != mSplash[0])
			path += mSplash;

		std::string::size_type f = path.find(mSplash[0], 0);
		while (f != path.npos)
		{
			int ret;
			if ((ret = CreateDirectoryInner(StringHelper::Left(path, f))) < 0)
				return ret;
			f = path.find(mSplash[0], f + 1);
		}

		return IsDirectoryExists(dir) ? 0 : -E_UNKNOWNERROR;
	}

	int zlibHelper::unzipToDirectory(const std::string &destDir, 
		const std::string &srcFile, const UnzipParam *param)
	{
		int r = 0, n;
		char strFileName[MAX_PATH];

		UnzipParam myParam;
		memset(&myParam, 0, sizeof(myParam));
		if (param)
			myParam = *param;

		if (destDir.empty())
			return -E_PARAMERROR;

		unzFile unzfile = ::unzOpen(srcFile.c_str());
		if (!unzfile)
			return -E_OPENFAIL;

		n = unzGoToFirstFile(unzfile);

		while (n == UNZ_OK)
		{
			if (::unzGetCurrentFileInfo(unzfile, NULL,
				strFileName, sizeof(strFileName),
				NULL, 0, NULL, 0) != UNZ_OK)
			{
				r = -E_GETFILEINFOFAIL;
				break;
			}

			std::string path = destDir, filepath;
			if (*path.rbegin() != mSplash[0])
				path += mSplash;
			path += strFileName;
			filepath = path;

			StringHelper::Replace(path, "/", mSplash);
			StringHelper::Replace(filepath, "/", mSplash);

			std::string::size_type f = path.rfind(mSplash[0]);

			if (f == path.size() - 1)
			{
				if (myParam.showVerb)
					fprintf(stdout, "Create directory: %s\n", strFileName);
				CreateDirectoryR(path);
			}
			else
			{
				if (myParam.showVerb)
					fprintf(stdout, "Unzip file: %s\n", filepath.c_str());

				if (f != path.npos)
					path.erase(f + 1);

				// 建立目录结构
				CreateDirectoryR(path);

				Handle<STDCFILEHandle> file = ::fopen(filepath.c_str(), "wb");
				if (!file) {
					r = -E_CREATEFILEFAIL;
					break;
				}

				if (::unzOpenCurrentFile(unzfile) != UNZ_OK)
				{
					r = false;
					break;
				}

				char buf[8192];
				int readed;
				while ((readed = unzReadCurrentFile(unzfile, buf, sizeof(buf))) > 0)
				{
					fwrite(buf, 1, readed, file);
				}
				::fclose(file);
				
				::unzCloseCurrentFile(unzfile);
			}

			n = ::unzGoToNextFile(unzfile);
		}

		::unzClose(unzfile);

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////

	int zlibHelper::zipDirectoryToFile(const std::string &destFile,
		const std::string &srcDirectory)
	{
		int r = 0;

		if (!IsDirectoryExists(srcDirectory.c_str()))
			return -E_OPENFAIL;

		zipFile zipFile = ::zipOpen(destFile.c_str(), APPEND_STATUS_CREATE);
		if (!zipFile)
			return -E_CREATEFILEFAIL;

		std::string dir = srcDirectory;
		if (StringHelper::Right(dir, 1) != mSplash)
			dir += mSplash;

		do {
			notstd::CFindIterator fi(dir, notstd::CFindIterator::FIFLAG_BREADTH_FIRST);
			for (notstd::CFindResult fr = fi.begin(); fr != fi.end(); fr = fi.next())
			{
				std::string filepath = fr.GetPath();
				if (fr.IsDirectory())
					continue;
				std::string relpath = StringHelper::Mid(filepath, dir.size());
				StringHelper::Replace(relpath, "\\", "/");

				if (!AddFile2Zip(filepath, relpath, zipFile)) {
					r = false;
					break;
				}

			}
		} while (0);

		::zipClose(zipFile, nullptr);

		return r;
	}

	bool zlibHelper::AddFile2Zip(const std::string &filePath, 
		const std::string &relpath, zipFile zipfile)
	{
		Handle<STDCFILEHandle> file = fopen(filePath.c_str(), "rb");
		if (!file)
			return false;
		printf("zip file %s\n", filePath.c_str());
		size_t readed;
		char buf[16384];
		zipOpenNewFileInZip(zipfile, relpath.c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr,
			Z_DEFLATED, Z_DEFAULT_COMPRESSION);
		while ((readed = fread(buf, 1, sizeof(buf), file)) > 0)
		{
			zipWriteInFileInZip(zipfile, buf, readed);
		}
		zipCloseFileInZip(zipfile);
		return true;
	}
}
