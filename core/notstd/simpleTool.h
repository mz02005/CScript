#pragma once
#include "config.h"

template <typename T>
class Help
{
public:
	static void Delete(T *p)
	{
		delete p;
	}

	static void Free(T *p)
	{
		::free(p);
	}
};

class NOTSTD_API STDCFILEHandle
{
public:
	typedef FILE* HandleType;

	static bool IsNullHandle(HandleType h);
	static void CloseHandleProc(HandleType h);
	static void SetHandleNull(HandleType &h);
};

class NOTSTD_API ZipFileHandle
{
public:
	typedef void* zipFile;
	typedef zipFile HandleType;

	static bool IsNullHandle(HandleType h);
	static void CloseHandleProc(HandleType h);
	static void SetHandleNull(HandleType &h);
};

class NOTSTD_API UnzipFileHandle
{
public:
	typedef void* unzFile;
	typedef unzFile HandleType;

	static bool IsNullHandle(HandleType h);
	static void CloseHandleProc(HandleType h);
	static void SetHandleNull(HandleType &h);
};

#ifdef PLATFORM_WINDOWS

class NOTSTD_API FileHandleType
{
public:
	typedef HANDLE HandleType;

	static bool IsNullHandle(HandleType h);
	static void CloseHandleProc(HandleType h);
	static void SetHandleNull(HandleType &h);
};

class NOTSTD_API NormalHandleType
{
public:
	typedef HANDLE HandleType;

	static bool IsNullHandle(HandleType h);
	static void CloseHandleProc(HandleType h);
	static void SetHandleNull(HandleType &h);
};

class NOTSTD_API ModuleType
{
public:
	typedef HMODULE HandleType;

	static bool IsNullHandle(HandleType h);
	static void CloseHandleProc(HandleType h);
	static void SetHandleNull(HandleType &h);
};

class NOTSTD_API FindFileHandle
{
public:
	typedef HANDLE HandleType;

	static bool IsNullHandle(HandleType h);
	static void CloseHandleProc(HandleType h);
	static void SetHandleNull(HandleType &h);
};

#endif

template <class T>
class Handle
{
public:
	typedef typename T::HandleType HandleType;

protected:
	HandleType mHandle;

public:
	Handle(HandleType h)
		: mHandle(h)
	{
	}

	Handle()
	{
		T::SetHandleNull(mHandle);
	}

	~Handle()
	{
		if (!T::IsNullHandle(mHandle))
		{
			T::CloseHandleProc(mHandle);
			T::SetHandleNull(mHandle);
		}
	}

	HandleType Detach() {
		HandleType r = mHandle;
		T::SetHandleNull(mHandle);
		return r;
	}

	operator bool() const {
		return !T::IsNullHandle(mHandle);
	}

	Handle& operator = (HandleType h)
	{
		Close();
		mHandle = h;
		return *this;
	}

	operator HandleType() {
		return mHandle;
	}

	operator HandleType() const {
		return mHandle;
	}

	HandleType* operator &() {
		return &mHandle;
	}

	void Close()
	{
		if (!T::IsNullHandle(mHandle))
		{
			T::CloseHandleProc(mHandle);
			T::SetHandleNull(mHandle);
		}
	}
};

#if defined(PLATFORM_WINDOWS)
template class NOTSTD_API Handle < NormalHandleType > ;
template class NOTSTD_API Handle < FileHandleType > ;
template class NOTSTD_API Handle < ModuleType > ;
template class NOTSTD_API Handle < ZipFileHandle > ;
template class NOTSTD_API Handle < STDCFILEHandle > ;
template class NOTSTD_API Handle < FindFileHandle > ;
template class NOTSTD_API Handle < UnzipFileHandle > ;
#endif

class NOTSTD_API Path
{
public:
#if defined(PLATFORM_WINDOWS)
	static const int mMaxPath = MAX_PATH;
#else
	static const int mMaxPath = 260;
#endif

	static bool CreateDirectoryRecursion(const std::string &path);

	typedef void (*OnPathName)(void *userData, const std::string &pathName, bool isDir);
	static void TraverseDirectory(const std::string &dirName, void *userData, OnPathName onPathName);
};

class NOTSTD_API zlibHelper
{
public:
	static int CompressDirectory(const std::wstring &src, const std::wstring &dest);
	static int UncompressDirectory(const std::wstring &zipFilePath, const std::wstring &destDirName);
};

class NOTSTD_API Time
{
public:
#if defined(PLATFORM_WINDOWS)
	typedef __time64_t TimeType;
#else
	typedef int64_t TimeType;
#endif
	TimeType mTime;

public:
	Time();
	Time(TimeType t);
#if defined(PLATFORM_WINDOWS)
	Time(FILETIME *filetime);
#endif
	Time(int year, int month, int day, int hour, int minute, int second);

	std::string GetString() const;

	static Time GetCurrentTime();
};

struct FileAttribute
{
	uint64_t fileAttributes;
	Time creationTime;
	Time lastAccessTime;
	Time lastWrittenTime;
	uint64_t fileSize;
};

class NOTSTD_API File
{
public:
	static int GetFileAttribute(const std::wstring &filePathName, FileAttribute *fileAttributes);
};
