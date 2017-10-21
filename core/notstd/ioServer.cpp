#include "stdafx.h"
#include "ioServer.h"

namespace notstd {

	///////////////////////////////////////////////////////////////////////////

	IOServerDataBuffer::IOServerDataBuffer()
		: mBegin(mBuf)
		, mData(nullptr)
		, mLen(0)
		, mCap(sizeof(mBuf))
	{
	}

	IOServerDataBuffer::~IOServerDataBuffer()
	{
		Clear();
		mLen = 0;
		mCap = sizeof(mBuf);
	}

	void IOServerDataBuffer::Clear()
	{
		if (mData)
		{
			free(mData);
			mData = nullptr;
		}
	}

	void IOServerDataBuffer::CopyData(const IOServerDataBuffer *other)
	{
		Clear();
		if (other->mData)
		{
			mData = reinterpret_cast<char*>(other->mCap);
			memcpy(mData, other->mData, other->mCap);
			mLen = other->mLen;
			mCap = other->mCap;
			mBegin = mData + (other->mBegin - other->mData);
		}
		else
		{
			memcpy(mBuf, other->mBuf, other->mLen);
			mLen = other->mLen;
			mCap = other->mCap;
			mBegin = mData + (other->mBegin - other->mBuf);
		}
	}

	void IOServerDataBuffer::SetBufferCap(size_t size)
	{
		if (size > mCap)
		{
			if (mData)
				free(mData);
			mCap = (size + 7) / 8 * 8;
			mData = (char*)malloc(mCap);
			mBegin = mData;
		}
		else
		{
			mBegin = mData ? mData : mBuf;
		}
		mLen = 0;
	}

	void IOServerDataBuffer::SetBufferSize(const void *buf, size_t size)
	{
		if (size > mCap)
		{
			if (mData)
				free(mData);
			mCap = (size + 7) / 8 * 8;
			mData = (char*)malloc(mCap);
			mBegin = mData;
		}
		else
		{
			mBegin = mData ? mData : mBuf;
		}
		memcpy(mBegin, buf, size);
		mLen = size;
	}

	////////////////////////////////////////////////////////////////////////////
	
	IOServerData::IOServerData(uint32_t iosType)
		: mType(iosType)
	{
#if defined(PLATFORM_WINDOWS)
		memset(static_cast<OVERLAPPED*>(this), 0, sizeof(OVERLAPPED));
#endif
	}

	IOServerData::~IOServerData()
	{
	}

	void IOServerData::SetHandle(HandleType h)
	{
		mHandleFunc = h;
	}

	void IOServerData::OnFunc(IOServer *ioServer, const IOErrorCode &e, IOServerData *data, size_t trans)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	SocketIOServerData::SocketIOServerData(uint32_t type)
		: IOServerData(type)
	{
	}

	SocketIOServerData::~SocketIOServerData()
	{
	}

	void SocketIOServerData::OnFunc(IOServer *ioServer, const IOErrorCode &e, IOServerData *data, size_t trans)
	{
		assert(data == static_cast<IOServerData*>(this));
		if (mHandleFunc)
			mHandleFunc(ioServer, e, data, trans);
	}

	///////////////////////////////////////////////////////////////////////////

	IOServerQuitMessageData::IOServerQuitMessageData()
		: IOServerData(IOS_QUIT)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	ReceiveIOServerData::ReceiveIOServerData()
		: SocketIOServerData(IOS_RECEIVE)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	SendIOServerData::SendIOServerData()
		: SocketIOServerData(IOS_SEND)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	ConnectIOServerData::ConnectIOServerData()
		: SocketIOServerData(IOS_CONNECT)
	{
	}

}
