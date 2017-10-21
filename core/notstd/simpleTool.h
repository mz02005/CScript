#pragma once
#include "config.h"

namespace notstd {
	NOTSTD_API uint32_t GetCurrentTick();
}

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

#if defined(PLATFORM_WINDOWS)
template <class T = NormalHandleType>
#else
template <class T>
#endif
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
	inline static char GetSplashCharacter()
	{
#if defined(PLATFORM_WINDOWS)
		return '\\';
#else
		return '/';
#endif
	}

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
	typedef time_t TimeType;
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

struct NOTSTD_API FileAttribute
{
#if defined(PLATFORM_WINDOWS)
	uint64_t fileAttributes;
#else
	mode_t fileAttributes;
#endif
	Time creationTime;
	Time lastAccessTime;
	Time lastWrittenTime;
	uint64_t fileSize;

	FileAttribute();

	bool isDir() const;
	bool isFile() const;
};

class NOTSTD_API File
{
public:
	static int GetFileAttribute(const std::string &filePathName, FileAttribute *fileAttributes);
};

#ifndef PLATFORM_WINDOWS
#define _TRUNCATE -1
inline int pi_ncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count)
{
	char *r = strncpy(strDest, strSource, numberOfElements);
	if (!r)
	{
		if (strDest)
			strDest[0] = 0;
		return EINVAL;
	}
	strDest[numberOfElements - 1] = 0;
	return 0;
}
#else
inline errno_t pi_ncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count)
{
	return strncpy_s(strDest, numberOfElements, strSource, count);
}
#endif

namespace notstd {
	NOTSTD_API void* loadlibrary(const char *module);
	NOTSTD_API void* getprocaddress(void *module, const char *name);
	NOTSTD_API bool freelibrary(void *module);

	class NOTSTD_API WinProfile
	{
	private:
		HANDLE mInnerInfo;

	public:
		enum {
			E_FERROR = 1,
			E_INVALID_SECFORMAT,
			E_SEC_DUMP,
			E_INVALID_VALUE,
			E_EMPTY_SECTION,
			E_VALUE_DUMP,
		};

		WinProfile();
		virtual ~WinProfile();

		int OpenProfile(const char *profileName);
		void CloseProfile();

		std::string _GetProfileString(const char *secName, const char *keyName, const std::string &defVal);
		int32_t _GetProfileInt32(const char *secName, const char *keyName, int32_t defVal);
		uint32_t _GetProfileUint32(const char *secName, const char *keyName, uint32_t defVal);
	};
}
