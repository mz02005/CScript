#include "stdafx.h"
#include "simpleTool.h"
#include "charconv.h"
#include "libzlib/zlib-1.2.8/contrib/minizip/zip.h"
#include "libzlib/zlib-1.2.8/contrib/minizip/unzip.h"
#include <assert.h>
#include "notstd/stringHelper.h"

///////////////////////////////////////////////////////////////////////////////

bool STDCFILEHandle::IsNullHandle(HandleType h)
{
	return (NULL == h);
}

void STDCFILEHandle::CloseHandleProc(HandleType h)
{
	::fclose(h);
}

void STDCFILEHandle::SetHandleNull(HandleType &h)
{
	h = NULL;
}

bool ZipFileHandle::IsNullHandle(HandleType h)
{
	return (NULL == h);
}

void ZipFileHandle::CloseHandleProc(HandleType h)
{
	::zipClose(h, NULL);
}

void ZipFileHandle::SetHandleNull(HandleType &h)
{
	h = NULL;
}

bool UnzipFileHandle::IsNullHandle(HandleType h)
{
	return (NULL == h);
}

void UnzipFileHandle::CloseHandleProc(HandleType h)
{
	::unzClose(h);
}

void UnzipFileHandle::SetHandleNull(HandleType &h)
{
	h = NULL;
}

#ifdef PLATFORM_WINDOWS

bool FileHandleType::IsNullHandle(HandleType h)
{
	return (h == INVALID_HANDLE_VALUE);
}

void FileHandleType::CloseHandleProc(HandleType h)
{
	::CloseHandle(h);
}

void FileHandleType::SetHandleNull(HandleType &h)
{
	h = INVALID_HANDLE_VALUE;
}

bool NormalHandleType::IsNullHandle(HandleType h)
{
	return (h == NULL);
}

void NormalHandleType::CloseHandleProc(HandleType h)
{
	::CloseHandle(h);
}

void NormalHandleType::SetHandleNull(HandleType &h)
{
	h = NULL;
}

bool ModuleType::IsNullHandle(HandleType h)
{
	return (h == NULL);
}

void ModuleType::CloseHandleProc(HandleType h)
{
	::FreeLibrary(h);
}

void ModuleType::SetHandleNull(HandleType &h)
{
	h = NULL;
}

bool FindFileHandle::IsNullHandle(HandleType h)
{
	return (NULL == h);
}

void FindFileHandle::CloseHandleProc(HandleType h)
{
	::FindClose(h);
}

void FindFileHandle::SetHandleNull(HandleType &h)
{
	h = NULL;
}
#endif

///////////////////////////////////////////////////////////////////////////////

static const char splash =
#ifdef _WINDOWS_
'\\';
#else
'/';
#endif

static bool AddFile2Zip(const char *strFilePath, const char *strRelPath, zipFile zipfile)
{
	Handle<STDCFILEHandle> file = fopen(strFilePath, "rb");
	if (!file)
		return false;

	int readed;
	char buf[8192];
	zipOpenNewFileInZip(zipfile, strRelPath, NULL, NULL, 0, NULL, 0, NULL,
		Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	while ((readed = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), file)) > 0)
	{
		zipWriteInFileInZip(zipfile, buf, readed);
	}
	zipCloseFileInZip(zipfile);
	return true;
}

#if defined(PLATFORM_WINDOWS)
bool Path::CreateDirectoryRecursion(const std::string &path)
{
	const std::wstring thePath = notstd::ICONVext::mbcsToUnicode(path);
	std::size_t s = 0, f;
	while ((f = thePath.find(splash, s)) != thePath.npos)
	{
		::CreateDirectoryW(thePath.substr(0, f).c_str(), NULL);
		s = f + 1;
	}
	if (s != path.size())
		::CreateDirectoryW(thePath.c_str(), NULL);
	return true;
}

void Path::TraverseDirectory(const std::string &dirName, void *userData, OnPathName onPathName)
{
	assert(onPathName);

	std::string toFind = dirName;
	if (toFind.empty())
		return;
	if (toFind.back() != '\\')
		toFind += '\\';
	
	typedef std::list<std::string> DirList;
	DirList theDirList;
	theDirList.push_back(toFind);

	while (theDirList.size())
	{
		std::string findDir = theDirList.front();
		theDirList.pop_front();

		(*onPathName)(userData, findDir, true);
		toFind = findDir + "*.*";

		WIN32_FIND_DATAA findData;
		Handle<FindFileHandle> findFile = ::FindFirstFileA(toFind.c_str(), &findData);
		if (findFile)
		{
			do {
				std::string newFile = findDir + findData.cFileName;

				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (strcmp(findData.cFileName, ".") == 0
						|| strcmp(findData.cFileName, "..") == 0)
						continue;

					theDirList.push_back(newFile + "\\");
				}
				else
				{
					onPathName(userData, newFile, false);
				}
			} while (::FindNextFileA(findFile, &findData));
		}
	}
}
#else
bool Path::CreateDirectoryRecursion(const std::string &path)
{
	return false;
}
void Path::TraverseDirectory(const std::string &dirName, void *userData, OnPathName onPathName)
{
}
#endif

int zlibHelper::UncompressDirectory(const std::wstring &zipFilePath, const std::wstring &destDirName)
{
	int n;
	char strFileName[Path::mMaxPath];
	int r = -2;

	if (destDirName.empty())
		return r;

	std::string theZipFilePath = notstd::ICONVext::unicodeToMbcs(zipFilePath);
	std::string theDestDir = notstd::ICONVext::unicodeToMbcs(destDirName);

	Handle<UnzipFileHandle> unzfile = ::unzOpen(theZipFilePath.c_str());
	if (!unzfile)
		return -3;

	n = unzGoToFirstFile(unzfile);

	r = 0;
	while (n == UNZ_OK)
	{
		if (::unzGetCurrentFileInfo(unzfile, NULL,
			strFileName, sizeof(strFileName),
			NULL, 0, NULL, 0) != UNZ_OK)
		{
			r = -4;
			break;
		}

		std::string path = theDestDir;
		if (path[path.size() - 1] != splash)
			path += splash;
		path += strFileName;

		//HWFC::CStringA path = strDest, filepath;
		//if (path.Right(1) != splash)
		//	path += splash;
		//path += strFileName;
		//filepath = path;

#ifdef PLATFORM_WINDOWS
		// 将path中的分隔符替换成相应平台的形式
		std::replace_if(path.begin(), path.end(), [](char c)->bool{
			return c == '/';
		}, '\\');
#endif

		// 由于最后一个分隔符之后的名称是文件名，所以为了建立目录，需排除最后的名字部分
		std::size_t f = path.rfind(splash);
		
		// 建立目录结构
		Path::CreateDirectoryRecursion(f == path.npos ? path : path.substr(0, f));

		Handle<STDCFILEHandle> file = ::fopen(path.c_str(), "wb");
		if (!file) {
			r = -5;
			break;
		}
		
		if (::unzOpenCurrentFile(unzfile) != UNZ_OK)
		{
			r = -6;
			break;
		}

		char buf[8192];
		int readed;
		while ((readed = unzReadCurrentFile(unzfile, buf, sizeof(buf))) > 0)
		{
			::fwrite(buf, 1, readed, file);
		}
		file.Close();

		::unzCloseCurrentFile(unzfile);

		n = ::unzGoToNextFile(unzfile);
	}

	return r;
}

#if defined(PLATFORM_WINDOWS)
int zlibHelper::CompressDirectory(const std::wstring &src, const std::wstring &dest)
{
	std::string theSource, theDest;
	theSource = notstd::ICONVext::unicodeToMbcs(src);
	theDest = notstd::ICONVext::unicodeToMbcs(dest);

	Handle<ZipFileHandle> zipfile = ::zipOpen(theDest.c_str(), APPEND_STATUS_CREATE);
	if (!zipfile)
		return -10;

	if (theSource.empty())
		return -11;
	if (theSource[theSource.size() - 1] != splash)
		theSource += splash;

	typedef std::list<std::string> ToDoDirList;
	ToDoDirList toDoDirList;
	
	WIN32_FIND_DATAA findData;
	toDoDirList.push_back(theSource);

	while (!toDoDirList.empty())
	{
		const std::string findDir = toDoDirList.front();
		toDoDirList.pop_front();

		std::string toFind = findDir + "*.*";

		Handle<FindFileHandle> findFile = ::FindFirstFileA(toFind.c_str(), &findData);
		if (findFile)
		{
			do {
				std::string newFile = findDir + findData.cFileName;

				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (strcmp(findData.cFileName, ".") == 0
						|| strcmp(findData.cFileName, "..") == 0)
						continue;

					toDoDirList.push_back(newFile + "\\");
				}
				else
				{
					std::string zipName = newFile;

					// 替换掉反斜杠
					std::replace_if(zipName.begin(), zipName.end(),
						[](char c)-> bool
					{
						return c == '\\';
					}, '/');
					// 去掉前面的路径部分
					zipName.erase(0, theSource.size());

					// 加入压缩包
					if (!AddFile2Zip(newFile.c_str(), zipName.c_str(), zipfile))
						return false;
				}
			} while (::FindNextFileA(findFile, &findData));
		}
	}

	return true;
}
#else
int zlibHelper::CompressDirectory(const std::wstring &src, const std::wstring &dest)
{
	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////

Time::Time()
	: mTime(0)
{
}

Time::Time(Time::TimeType t)
	: mTime(t)
{
}

#if defined(PLATFORM_WINDOWS)
Time::Time(FILETIME *filetime)
{
	SYSTEMTIME st;
	if (::FileTimeToSystemTime(filetime, &st))
	{
		SYSTEMTIME lt;
		::SystemTimeToTzSpecificLocalTime(NULL, &st, &lt);
		*this = Time(lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
	}
	else
	{
		assert(0);
	}
}
#endif

Time::Time(int year, int month, int day, int hour, int minute, int second)
{
	tm t = { 0 };
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
#if defined(PLATFORM_WINDOWS)
	mTime = _mktime64(&t);
#else
	assert(0);
	mTime = mktime(&t);
#endif

	assert(mTime != (TimeType)-1);
}

std::string Time::GetString() const
{
	tm *t = localtime(&mTime);
	if (!t)
		return "";
	std::string r;
	return StringHelper::Format(r, "%02d-%02d-%02d %02d:%02d:%02d",
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);
}

Time Time::GetCurrentTime()
{
#if defined(PLATFORM_WINDOWS)
	return Time(_time64(NULL));
#else
	return Time(time(NULL));
#endif
}

///////////////////////////////////////////////////////////////////////////////

int File::GetFileAttribute(const std::wstring &filePathName, FileAttribute *fileAttributes)
{
#if defined(PLATFORM_WINDOWS)
	WIN32_FILE_ATTRIBUTE_DATA win32FileAttributeData;
	if (!::GetFileAttributesExW(filePathName.c_str(), GetFileExInfoStandard, &win32FileAttributeData))
		return -1;
	fileAttributes->fileAttributes = win32FileAttributeData.dwFileAttributes;
	fileAttributes->creationTime = Time(&win32FileAttributeData.ftCreationTime);
	fileAttributes->lastAccessTime = Time(&win32FileAttributeData.ftLastAccessTime);
	fileAttributes->lastWrittenTime = Time(&win32FileAttributeData.ftLastWriteTime);
	fileAttributes->fileSize = (static_cast<uint64_t>(win32FileAttributeData.nFileSizeHigh) << 32)
		+ win32FileAttributeData.nFileSizeLow;
	return 0;
#else
	return -1;
#endif
}
