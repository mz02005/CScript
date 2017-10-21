#pragma once

#include "httpInterface.h"
#include <list>

class HttpDataBuffer : public IDataBuffer
{
	std::list<IBufferPiece*> mPieceList;
	std::list<IBufferPiece*>::iterator mIter;

	uint64_t mTotalSize;

public:
	HttpDataBuffer();
	virtual ~HttpDataBuffer();
	virtual IBufferPiece* CreatePeice();
	virtual int AppendPiece(IBufferPiece *piece);
	virtual int InsertPieceBeforeHeader(IBufferPiece *piece);
	virtual IBufferPiece* RemoveHeaderPiece();
	virtual IBufferPiece* RemoveTailPiece();
	virtual void RemoveAll();
	virtual int GetFirstPiece();
	virtual int GetNextPiece(IBufferPiece *&piece);
	virtual uint64_t GetTotalSize();
};
