#include "stdafx.h"
#include "filesystem.h"
#include <assert.h>
#include <string.h>

namespace notstd {
	///////////////////////////////////////////////////////////////////////////

#if defined(PLATFORM_WINDOWS)
	const char AppHelper::splash[] = "\\";
#else
	const char AppHelper::splash[] = "/";
#endif

#if defined(PLATFORM_WINDOWS)
	std::string AppHelper::GetAppPath()
	{
		std::string appDir;
		appDir.resize(MAX_PATH);
		appDir.resize(::GetModuleFileNameA(NULL, &appDir[0], MAX_PATH));
		return appDir;
	}
#else
	std::string AppHelper::GetAppPath()
	{
		std::string appDir;
		const static int MAX_PATH_LEN = 1024;
		appDir.resize(MAX_PATH_LEN);
		int c = readlink("/proc/self/exe", &appDir[0], MAX_PATH_LEN);
		if (c < 0 || c >= MAX_PATH_LEN)
			return "";
		appDir.resize(c);
		return appDir;
}
#endif

	///////////////////////////////////////////////////////////////////////////

	const CFindResult CFindResult::mFindEnd;

	CFindResult::CFindResult()
		: mIsDirectory(false)
		, mPathName(new std::string)
	{
	}

	CFindResult::CFindResult(const std::string &pathName, bool isDir)
		: mIsDirectory(isDir)
		, mPathName(new std::string(pathName))
	{
	}

	CFindResult::CFindResult(const CFindResult& fr)
		: mPathName(new std::string)
	{
		*this = fr;
	}

	CFindResult::~CFindResult()
	{
		delete mPathName;
	}

	std::string CFindResult::GetPath() const
	{
		return *mPathName;
	}

	std::string CFindResult::GetName() const
	{
		std::string::size_type f = mPathName->rfind(CFindIterator::mSplitChar);
		if (f != mPathName->npos)
			return mPathName->substr(f + 1);
		return *mPathName;
	}

	std::string CFindResult::GetTitle() const
	{
		std::string name = GetName();
		std::string::size_type f = name.rfind('.');
		if (f != name.npos)
			return name.substr(0, f);
		return name;
	}

	std::string CFindResult::GetSuffix() const
	{
		if (mIsDirectory)
			return "";

		std::string name = GetName();
		std::string::size_type f = name.rfind('.');
		if (f != name.npos)
			return name.substr(f + 1);
		return "";
	}
	
	bool CFindResult::operator == (const CFindResult &fr)
	{
		return *mPathName == *fr.mPathName;
	}

	bool CFindResult::operator != (const CFindResult &fr)
	{
		return *mPathName != *fr.mPathName;
	}

	CFindResult& CFindResult::operator = (const CFindResult &fr)
	{
		mIsDirectory = fr.mIsDirectory;
		*mPathName = *fr.mPathName;
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CFindIterator::CFindIterator(const std::string &path, CIteratorType it)
		: mIterator(it)
		, mIteratorContextRecord(new CIteratorContextRecord)
		, mInitPath(new std::string)
	{
		assert
			(
			it >= FIFLAG_NULL
			&& it <= FIFLAG_BREADTH_FIRST
			);

		*mInitPath = path;
	}

	CFindIterator::CFindIterator(const CFindIterator &fi)
		: mInitPath(new std::string)
	{
		*this = fi;
	}

	CFindIterator::~CFindIterator ()
	{
		while (!mIteratorContextRecord->empty())
		{
			ReleaseIteratorContext(mIteratorContextRecord->front());
			mIteratorContextRecord->pop_front();
		}
		delete mInitPath;
	}

	CFindIterator::CIteratorContext* CFindIterator::CreateIteratorContext(const std::string &dir)
	{
		CIteratorContext *context = new CIteratorContext;
		context->dir = dir;
		if (dir.size() && *dir.rbegin() != mSplitChar)
			context->dir += mSplitChar;
		
#if defined(PLATFORM_WINDOWS)
		context->hFindFile = INVALID_HANDLE_VALUE;
#else
		context->hFindFile = NULL;
#endif

		return context;
	}

	void CFindIterator::ReleaseIteratorContext(CIteratorContext *context)
	{
#if defined(PLATFORM_WINDOWS)
		if (context->hFindFile != INVALID_HANDLE_VALUE)
		{
			::FindClose(context->hFindFile);
			context->hFindFile = INVALID_HANDLE_VALUE;
		}
#else
		if (context->hFindFile)
		{
			::closedir(context->hFindFile);
			context->hFindFile = NULL;
		}
#endif

		delete context;
	}

	char CFindIterator::mSplitChar = 
#ifdef PLATFORM_WINDOWS
		'\\';
#else
		'/';
#endif

#ifdef PLATFORM_WINDOWS
#pragma warning (push)
#pragma warning (disable: 4800)
#endif

	// 深度优先操作使用一个堆栈保存数据，
	// 新的堆栈数据放在列表的前面
	// 广度优先操作使用一个队列保存数据
	// 新的队列数据放在列表的最后
	CFindResult CFindIterator::begin()
	{
		// 保留最初的扫描目录信息
		switch (mIterator)
		{
		case FIFLAG_NULL:
		case FIFLAG_DEPTH_FIRST:
		case FIFLAG_BREADTH_FIRST:
			while (!mIteratorContextRecord->empty())
			{
				ReleaseIteratorContext(mIteratorContextRecord->front());
				mIteratorContextRecord->pop_front();
			}
			mIteratorContextRecord->push_back(CreateIteratorContext(*mInitPath));
			break;
		}

		return FindInternal();
	}

	CFindResult CFindIterator::next()
	{
		return FindInternal();
	}

	CFindIterator::CIteratorContext* CFindIterator::GetIteratorContext()
	{
		if (mIteratorContextRecord->empty())
			return NULL;

		switch(mIterator)
		{
		case FIFLAG_NULL:
		case FIFLAG_BREADTH_FIRST:
		case FIFLAG_DEPTH_FIRST:
			return mIteratorContextRecord->front();
		}

		return NULL;
	}

	bool CFindIterator::RemoveFirstContext()
	{
		if (mIteratorContextRecord->empty())
			return false;

		switch(mIterator)
		{
		case FIFLAG_BREADTH_FIRST:
		case FIFLAG_DEPTH_FIRST:
			ReleaseIteratorContext(mIteratorContextRecord->front());
			mIteratorContextRecord->pop_front();
			return true;

		case FIFLAG_NULL:
			return false;
		}

		return false;
	}

#ifdef PLATFORM_WINDOWS
	CFindResult CFindIterator::FindInternal()
	{
		WIN32_FIND_DATAA ffd;
		CIteratorContext *pIterCont, *pIterContCreat;
		std::string strPathName;
		bool bFind;

		CFindResult fr = end();

		do
		{
			pIterCont = GetIteratorContext();
			if (!pIterCont)
				break;
			bFind = false;
			if (pIterCont->hFindFile == INVALID_HANDLE_VALUE)
			{
				pIterCont->hFindFile = ::FindFirstFileA(
					(pIterCont->dir + std::string("*.*")).c_str (), 
					&ffd);
				if (pIterCont->hFindFile != INVALID_HANDLE_VALUE)
					bFind = true;
			}
			else
			{
				bFind = !!::FindNextFileA(
					pIterCont->hFindFile, &ffd);
			}

			if (bFind)
			{
				bool bDir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 
					true : false;

				strPathName = pIterCont->dir + ffd.cFileName;
				if (bDir)
				{
					if (strcmp (ffd.cFileName, ".") == 0 || 
						strcmp (ffd.cFileName, "..") == 0)
						continue;

					if (mIterator == FIFLAG_BREADTH_FIRST)
					{
						pIterContCreat = CreateIteratorContext(strPathName);
						mIteratorContextRecord->push_back(pIterContCreat);
					}
					if (mIterator == FIFLAG_DEPTH_FIRST)
					{
						pIterContCreat = CreateIteratorContext(strPathName);
						mIteratorContextRecord->push_front(pIterContCreat);
					}
				}

				return CFindResult(strPathName, bDir);
			}
			else
			{
				if (!RemoveFirstContext())
					break;
				else
					continue;
			}
		} while (1);

		return end ();
	}
#else
	// linux代码
	CFindResult CFindIterator::FindInternal()
	{
		struct dirent *pDirent;
		struct stat filestat;
		CIteratorContext *pIterCont, *pIterContCreat;
		std::string strPathName;
		bool bFind;

		CFindResult fr = end();

		do
		{
			pIterCont = GetIteratorContext();
			if (!pIterCont)
				break;
			bFind = false;
			if (pIterCont->hFindFile == NULL)
			{
				pIterCont->hFindFile = ::opendir(pIterCont->dir.c_str ());
				if (pIterCont->hFindFile)
				{
					pDirent = ::readdir(
						pIterCont->hFindFile);
					if (pDirent)
						bFind = true;
					else
						::closedir(pIterCont->hFindFile);
				}
			}
			else
			{
				pDirent = readdir(pIterCont->hFindFile);
				bFind = (pDirent ? true : false);
			}

			if (bFind)
			{
				strPathName = pIterCont->dir + pDirent->d_name;
				stat(strPathName.c_str (), &filestat);
				bool bDir = (S_ISDIR(filestat.st_mode)) ? true : false;

				if (bDir)
				{
					if (strcmp(pDirent->d_name, ".") == 0 ||
						strcmp(pDirent->d_name, "..") == 0)
						continue;

					if (mIterator == FIFLAG_BREADTH_FIRST)
					{
						pIterContCreat = CreateIteratorContext(strPathName);
						mIteratorContextRecord->push_back(pIterContCreat);
					}
					if (mIterator == FIFLAG_DEPTH_FIRST)
					{
						pIterContCreat = CreateIteratorContext(strPathName);
						mIteratorContextRecord->push_front(pIterContCreat);
					}
				}

				return CFindResult(strPathName, bDir);
			}
			else
			{
				if (!RemoveFirstContext())
					break;
				else
					continue;
			}
		} while (1);

		return end();
	}
#endif

#ifdef PLATFORM_WINDOWS
#pragma warning(pop)
#endif

	CFindResult CFindIterator::end()
	{
		return CFindResult::mFindEnd;
	}

	CFindIterator& CFindIterator::operator = (const CFindIterator &fi)
	{
		assert(false);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////////
}
