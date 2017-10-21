#include "stdafx.h"
#include "dataBufferImpl.h"

class SimpleHttpPiece : public IBufferPiece
{
	std::string mBuf;

public:
	virtual int AppendData(const void *buf, size_t s)
	{
		mBuf.append(reinterpret_cast<const char*>(buf), s);
		return 0;
	}

	virtual int GetBuffer(void *&buf, size_t &s)
	{
		buf = &mBuf[0];
		s = mBuf.size();
		return 0;
	}

	virtual uint64_t GetSize()
	{
		return mBuf.size();
	}
};

IBufferPiece* HttpDataBuffer::CreatePeice()
{
	return static_cast<IBufferPiece*>(CreateRefObject<SimpleHttpPiece>());
}

HttpDataBuffer::HttpDataBuffer()
	: mTotalSize(0)
{
}

HttpDataBuffer::~HttpDataBuffer()
{
	for (auto iter = mPieceList.begin(); iter != mPieceList.end(); iter++)
		(*iter)->DecRef();
	mPieceList.clear();
}

int HttpDataBuffer::AppendPiece(IBufferPiece *piece)
{
	mPieceList.push_back(piece);
	mTotalSize += piece->GetSize();
	return 0;
}

int HttpDataBuffer::InsertPieceBeforeHeader(IBufferPiece *piece)
{
	mPieceList.push_front(piece);
	mTotalSize += piece->GetSize();
	return 0;
}

IBufferPiece* HttpDataBuffer::RemoveHeaderPiece()
{
	if (mPieceList.empty())
		return nullptr;
	IBufferPiece *r = static_cast<IBufferPiece*>(mPieceList.front());
	mTotalSize -= r->GetSize();
	mPieceList.pop_front();
	return r;
}

IBufferPiece* HttpDataBuffer::RemoveTailPiece()
{
	if (mPieceList.empty())
		return nullptr;
	IBufferPiece *r = static_cast<IBufferPiece*>(mPieceList.back());
	mTotalSize -= r->GetSize();
	mPieceList.pop_back();
	return r;
}

void HttpDataBuffer::RemoveAll()
{
	for (auto &piece : mPieceList)
		piece->DecRef();
	mPieceList.clear();
	mTotalSize = 0;
}

int HttpDataBuffer::GetFirstPiece()
{
	mIter = mPieceList.begin();
	return mIter == mPieceList.end() ? -1 : 0;
}

int HttpDataBuffer::GetNextPiece(IBufferPiece *&piece)
{
	if (mIter == mPieceList.end())
		return -1;
	piece = *mIter++;
	return 0;
}

uint64_t HttpDataBuffer::GetTotalSize()
{
	return mTotalSize;
}

