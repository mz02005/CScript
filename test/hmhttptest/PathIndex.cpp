#include "stdafx.h"
#include "hmhttptest.h"
#include "notstd/notstd.h"
#include <fstream>

int PathIndex::LoadTableFromHttpTest(httptest &httpTest)
{
	mPathTable.clear();
	auto &theList = httpTest.mReleaseList.mAddList.GetList();
	mPathTable.reserve(theList.GetCount());
	POSITION pos = theList.GetHeadPosition();
	while (pos)
	{
		auto item = theList.GetNext(pos);
		if (!item->isInheritFrom(OBJECT_INFO(add)))
			continue;
		auto theAdd = static_cast<add*>(item);
		mPathTable.push_back(std::make_pair(theAdd->mUrl.mVal,
			std::make_tuple(theAdd->mLocal.mVal, theAdd->mUsername.mVal,
			theAdd->mPassword.mVal, &theAdd->mRealExtWhiteList, &theAdd->mRealExtBlackList)));
		if (notstd::StringHelper::Right(std::get<0>(mPathTable.back().second), 1) != "/")
			std::get<0>(mPathTable.back().second) += "\\";
	}
	return 0;
}

int PathIndex::GetPathIndex(const std::string &url) const
{
	for (size_t i = 0; i < mPathTable.size(); i++)
	{
		if (notstd::StringHelper::Left(url, mPathTable[i].first.size()) == mPathTable[i].first)
		{
			if (url.size() > mPathTable[i].first.size()
				&& url[mPathTable[i].first.size()] != '/')
				continue;

			return i + 1;
		}
	}
	return -1;
}

