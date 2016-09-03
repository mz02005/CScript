#include "stdafx.h"
#include "ioServer.h"
#include <Mswsock.h>

namespace notstd {
	///////////////////////////////////////////////////////////////////////////

	notstdException::notstdException()
		: mBuf(new char[128])
		, mLen(128)
	{
		mBuf[0] = 0;
	}

	notstdException::notstdException(const char *what)
	{
		mLen = strlen(what) + 1;
		mBuf = new char[mLen];
		memcpy(mBuf, what, mLen);
	}

	char* notstdException::adjustRoom(size_t s)
	{
		if (s >= mLen)
		{
			delete[] mBuf;
			mBuf = new char[s];
			mLen = s;
		}
		return mBuf;
	}

	const char* notstdException::what() const
	{
		return mBuf;
	}

	winSystemError::winSystemError()
		: mError(::GetLastError())
	{
		FormatErrorMessage();
	}

	winSystemError::winSystemError(DWORD errCode)
		: mError(errCode)
	{
		FormatErrorMessage();
	}

	void winSystemError::FormatErrorMessage()
	{
		DWORD s;
		LPSTR msg = nullptr;
		if ((s = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr, mError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, nullptr)) > 0)
		{
			if (s >= mLen)
			{
				if (adjustRoom(s + 1))
				{
					memcpy(mBuf, msg, s + 1);
				}
				else
				{
					memcpy(mBuf, msg, mLen - 1);
					mBuf[mLen] = 0;
				}
			}
			else
				memcpy(mBuf, msg, s + 1);
			::LocalFree(msg);
		}
	}

	///////////////////////////////////////////////////////////////////////////

	IOServer::IOServer()
	{
	}

	IOServer::~IOServer()
	{
		DestroyIOServer();
	}

	void IOServer::CreateIOServer()
	{
		mIOServer = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (!mIOServer)
			throw winSystemError();
	}

	void IOServer::DestroyIOServer()
	{
		if (mIOServer)
			mIOServer.Close();
	}

	void IOServer::BindToHandle(HANDLE h)
	{
		bool r = !!::CreateIoCompletionPort(
			h, mIOServer, NULL, 0);
		if (!r)
			throw winSystemError();
	}

	void IOServer::PostUserEvent(IOServerData *data)
	{
		::PostQueuedCompletionStatus(mIOServer, 0, NULL, static_cast<LPOVERLAPPED>(data));
	}

	void IOServer::PostQuit()
	{
		::PostQueuedCompletionStatus(mIOServer, 0, NULL, 
			static_cast<LPOVERLAPPED>(&mQuitData));
	}

	bool IOServer::RunOneStep()
	{
		ULONG_PTR completionKey;
		DWORD transferred;
		OVERLAPPED *ol;

		BOOL r = ::GetQueuedCompletionStatus(mIOServer, &transferred, &completionKey,
			&ol, INFINITE);
		if (!r && !ol)
			return false;

		IOErrorCode ec(!!r);
		IOServerData *ioData = static_cast<IOServerData*>(ol);
		if (ioData)
		{
			if (IOServerData::IOS_QUIT == ioData->mType)
				return false;

			switch (ioData->mType)
			{
			case IOServerData::IOS_SEND:
			case IOServerData::IOS_RECEIVE:
				// 链接关闭了
				if (!transferred)
					ioData->OnFunc(this, false, ioData, 0);
				else
					ioData->OnFunc(this, ec, ioData, transferred);
				break;

			case IOServerData::IOS_ACCEPT:
			case IOServerData::IOS_CONNECT:
				if (ioData->mType == IOServerData::IOS_ACCEPT)
				{
					AcceptIOServerData *aiod = static_cast<AcceptIOServerData*>(ioData);

					INT serverLen, clientLen;

					if (::setsockopt(aiod->mSock, SOL_SOCKET,
						SO_UPDATE_ACCEPT_CONTEXT,
						reinterpret_cast<const char*>(&aiod->mServerSocket),
						sizeof(SOCKET)) != 0)
						break;

					(*NetEnviroment::GetAcceptExSockaddrs)(aiod->mSockAddr,
						0, sizeof(NetAddress) + 16, sizeof(NetAddress) + 16,
						reinterpret_cast<LPSOCKADDR*>(&aiod->mServer), &serverLen,
						reinterpret_cast<LPSOCKADDR*>(&aiod->mClient), &clientLen);
					::getsockname(aiod->mSock, reinterpret_cast<sockaddr*>(&aiod->mServer), &serverLen);
					::getpeername(aiod->mSock, reinterpret_cast<sockaddr*>(&aiod->mClient), &clientLen);
				}
				ioData->OnFunc(this, ec, ioData, transferred);
				break;

			default:
				// 对于其它情况，直接转发状态回调
				ioData->OnFunc(this, ec, ioData, transferred);
				break;
			}
		}

		return true;
	}

	void IOServer::Run()
	{
		for (;RunOneStep();)
		{
		}
	}
	
	///////////////////////////////////////////////////////////////////////////

	IOSocket::IOSocket(IOServer &ioServer)
		: mIOServer(ioServer)
	{
	}

	SOCKET IOSocket::Create(SocketType sockType, ProtocolType protocolType, DWORD flags)
	{
		SOCKET r = mSock.Create(sockType, protocolType, flags);
		if (r != INVALID_SOCKET)
		{
			mIOServer.BindToHandle((HANDLE)mSock.GetHandle());
		}
		return r;
	}

	int IOSocket::Close()
	{
		return mSock.Close();
	}

	void IOSocket::AsyncConnect(const NetAddress &addr, ConnectIOServerData *data)
	{
		mSock.Bind(notstd::NetAddress("0.0.0.0", 0));
		data->mSock = mSock.GetHandle();
		BOOL r = (*NetEnviroment::ConnectEx)(mSock.GetHandle(),
			reinterpret_cast<const sockaddr*>(&addr), sizeof(addr),
			nullptr, 0L, nullptr, static_cast<LPOVERLAPPED>(data));
		if (!r && ::GetLastError() != WSA_IO_PENDING)
			throw notstd::winSystemError();
	}

	void IOSocket::AsyncRecv(ReceiveIOServerData *data)
	{
		WSABUF buf;
		buf.buf = data->GetBufferAndCap().first;
		buf.len = data->GetBufferAndCap().second;
		DWORD flag = 0;
		int r = ::WSARecv(mSock.GetHandle(), &buf, 1, nullptr, &flag,
			static_cast<LPOVERLAPPED>(data), nullptr);
		if (r && ::GetLastError() != WSA_IO_PENDING)
			throw notstd::winSystemError();
	}

	void IOSocket::AsyncSend(SendIOServerData *data)
	{
		DWORD flag = 0;
		WSABUF buf;
		auto x = data->GetBufferAndSize();
		buf.buf = x.first;
		buf.len = x.second;
		int r = ::WSASend(mSock.GetHandle(), &buf, 1, nullptr, flag,
			static_cast<LPWSAOVERLAPPED>(data), nullptr);
		if (r && ::GetLastError() != WSA_IO_PENDING)
			throw notstd::winSystemError();
	}

	///////////////////////////////////////////////////////////////////////////

	AcceptSocket::AcceptSocket(IOServer &ioServer)
		: IOSocket(ioServer)
	{
	}

	void AcceptSocket::AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket)
	{
		DWORD recved;
		data->mSock = mSock.GetHandle();
		data->mServerSocket = listenSocket->GetHandle().GetHandle();
		if (!(*NetEnviroment::AcceptEx)(listenSocket->GetHandle().GetHandle(),
			mSock.GetHandle(),
			reinterpret_cast<PVOID>(data->mSockAddr), 0,
			sizeof(NetAddress) + 16,
			sizeof(NetAddress) + 16,
			&recved, static_cast<LPOVERLAPPED>(data)) && ::GetLastError() != ERROR_IO_PENDING)
			throw notstd::winSystemError();
	}

	///////////////////////////////////////////////////////////////////////////

	IOErrorCode::IOErrorCode(bool succeeded)
		: mSucceeded(succeeded)
		, mErrorCode(::GetLastError())
	{
	}

	IOErrorCode::operator bool() const
	{
		return !mSucceeded;
	}

	std::string IOErrorCode::what() const
	{
		notstd::winSystemError e(mErrorCode);
		return e.what();
	}

	///////////////////////////////////////////////////////////////////////////

	IOServerData::IOServerData(uint32_t iosType)
		: mType(iosType)
	{
		memset(static_cast<OVERLAPPED*>(this), 0, sizeof(OVERLAPPED));
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

	IOServerDataWithCache::IOServerDataWithCache(uint32_t t)
		: IOServerData(t)
		, mBegin(mBuf)
		, mData(nullptr)
		, mLen(0)
		, mCap(sizeof(mBuf))
	{
	}

	IOServerDataWithCache::~IOServerDataWithCache()
	{
		if (mData)
			free(mData);
	}

	void IOServerDataWithCache::SetSendBuffer(const void *d, size_t len)
	{
		if (len > mCap)
		{
			if (mData)
				free(mData);
			mCap = (len + 7) / 8 * 8;
			mData = (char*)malloc(mCap);
			mBegin = mData;
		}
		else
		{
			mBegin = mData ? mData : mBuf;
		}
		memcpy(mBegin, d, len);
		mLen = len;
	}

	std::pair<char*, size_t> IOServerDataWithCache::GetBufferAndSize()
	{
		return std::make_pair(mBuf, mLen);
	}

	std::pair<char*, size_t> IOServerDataWithCache::GetBufferAndCap()
	{
		return std::make_pair(mBuf, mCap);
	}

	///////////////////////////////////////////////////////////////////////////

	SocketIOServerData::SocketIOServerData(uint32_t type)
		: IOServerDataWithCache(type)
	{
	}

	void SocketIOServerData::OnFunc(IOServer *ioServer, const IOErrorCode &e, IOServerData *data, size_t trans)
	{
		if (mHandleFunc)
			mHandleFunc(ioServer, e, data, trans);
	}

	///////////////////////////////////////////////////////////////////////////

	IOServerQuitMessageData::IOServerQuitMessageData()
		: SocketIOServerData(IOS_QUIT)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	ConnectIOServerData::ConnectIOServerData()
		: SocketIOServerData(IOS_CONNECT)
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

	AcceptIOServerData::AcceptIOServerData()
		: SocketIOServerData(IOS_ACCEPT)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	TimerData::TimerData()
		: IOServerData(IOS_TIMER)
	{
	}

	void TimerData::OnFunc(IOServer *ioServer, const IOErrorCode &e,
		IOServerData *data, size_t trans)
	{
		mIOTimer->CloseTimer();

		if (mHandleFunc)
			mHandleFunc(ioServer, true, data, trans);

		mIOTimer->CreateTimer();
	}

	IOTimer::IOTimer(IOServer &ioServer)
		: mIOServer(ioServer)
		, mMMResult(NULL)
		, mDelay(1000)
	{
	}

	IOTimer::~IOTimer()
	{
		CloseTimer();
	}

	void IOTimer::CloseTimer()
	{
		if (mMMResult)
		{
			::timeKillEvent(mMMResult);
			mMMResult = NULL;
		}
	}

	void IOTimer::CreateTimer(uint32_t delay)
	{
		mDelay = delay;
		mTimeData.mIOTimer = this;
		mMMResult = ::timeSetEvent(delay, 10, &IOTimer::TimeCallBack,
			reinterpret_cast<DWORD_PTR>(this), TIME_ONESHOT | TIME_KILL_SYNCHRONOUS);
	}

	void IOTimer::CreateTimer()
	{
		mMMResult = ::timeSetEvent(mDelay, 10, &IOTimer::TimeCallBack,
			reinterpret_cast<DWORD_PTR>(this), TIME_ONESHOT | TIME_KILL_SYNCHRONOUS);
	}

	///////////////////////////////////////////////////////////////////////////
}
