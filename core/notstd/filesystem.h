#pragma once
#include "notstd/config.h"
#include <list>

namespace notstd {

class CFindResult;
class CFindIterator;

class NOTSTD_API CFindResult
{
	friend class CFindIterator;

private:
	std::string *mPathName;
	bool mIsDirectory;

	static const CFindResult mFindEnd;

public:
	CFindResult();
	CFindResult(const std::string &pathName, bool isDir = false);
	CFindResult(const CFindResult& fr);
	~CFindResult();

	std::string GetPath() const;
	std::string GetName() const;
	std::string GetTitle() const;
	std::string GetSuffix() const;
	bool IsDirectory() const { return mIsDirectory; }

	CFindResult& operator = (const CFindResult& fr);
	bool operator == (const CFindResult& fr);
	bool operator != (const CFindResult& fr);
};

class NOTSTD_API CFindIterator
{
	friend class CFindResult;

public:
	enum CIteratorType {
		FIFLAG_NULL,
		FIFLAG_DEPTH_FIRST,
		FIFLAG_BREADTH_FIRST,
	};

private:
	CIteratorType mIterator;

	struct CIteratorContext
	{
		std::string dir;

#ifdef PLATFORM_WINDOWS
		HANDLE hFindFile;
#else
		DIR *hFindFile;
#endif
	};

	typedef std::list<CIteratorContext*> CIteratorContextRecord;
	CIteratorContextRecord *mIteratorContextRecord;

	std::string *mInitPath;
	static char mSplitChar;

private:
	CIteratorContext* CreateIteratorContext(const std::string &dir);
	void ReleaseIteratorContext(CIteratorContext *context);

	CFindResult FindInternal();

	CIteratorContext* GetIteratorContext();
	bool RemoveFirstContext();

public:
	CFindIterator(const std::string &path,
		CIteratorType it = FIFLAG_NULL);
	CFindIterator(const CFindIterator &fi);
	~CFindIterator();

	CFindResult begin();
	CFindResult next();
	CFindResult end();

	CFindIterator& operator = (const CFindIterator &fi);
	CIteratorType GetIteratorType() const { return mIterator; }
};

}
