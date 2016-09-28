#include "stdafx.h"
#include "ioServer.h"
#include <Mswsock.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

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

	SSLContext::SSLContext()
		: mSSLCtx(nullptr)
	{
		OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, nullptr);
		const SSL_METHOD* meth = SSLv23_method();
		mSSLCtx = SSL_CTX_new(meth);
		//SSL_CTX_set_verify(mSSLCtx, SSL_VERIFY_NONE, nullptr);
	}

	SSLContext::~SSLContext()
	{
		if (mSSLCtx)
		{
			SSL_CTX_free(mSSLCtx);
			mSSLCtx = nullptr;
		}
	}

	SSL_CTX* SSLContext::GetSSLContext()
	{
		return mSSLCtx;
	}

	bool SSLContext::UsePrivateKeyFile(const char *filePathName, char *password)
	{
		if (password)
		{
			SSL_CTX_set_default_passwd_cb_userdata(mSSLCtx, 
				reinterpret_cast<void*>(password));
		}
		if (SSL_CTX_use_PrivateKey_file(mSSLCtx, filePathName, SSL_FILETYPE_PEM) <= 0
			|| !SSL_CTX_check_private_key(mSSLCtx))
			return false;
		return true;
	}

	bool SSLContext::UseCertificateFile(const char *filePathName)
	{
		if (SSL_CTX_use_certificate_file(mSSLCtx,
			filePathName, SSL_FILETYPE_PEM) <= 0)
			return false;
		return true;
	}

	bool SSLContext::SetNoVerify()
	{
		SSL_CTX_set_verify(mSSLCtx, SSL_VERIFY_NONE, nullptr);
		return true;
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
				{
					if (IOServerData::IOS_SEND == ioData->mType)
					{
						// 保证只有在用户要求发送的数据全部发送后，才回调用户指定的函数
						notstd::SendIOServerData *sndData =
							static_cast<notstd::SendIOServerData*>(ioData);
						auto buffer = sndData->GetDataBuffer();
						if (buffer->mLen > transferred)
						{
							buffer->mLen -= transferred;
							buffer->mBegin += transferred;
							IOSocket ioSocket(*this);
							ioSocket.GetHandle().Attach(sndData->mSock);
							ioSocket.AsyncSend(sndData);
							ioSocket.GetHandle().Detach();
							return true;
						}
					}
					ioData->OnFunc(this, ec, ioData, transferred);
				}
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

	AcceptIOServerData::AcceptIOServerData()
		: SocketIOServerData(IOS_ACCEPT)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	ConnectIOServerData::ConnectIOServerData()
		: SocketIOServerData(IOS_CONNECT)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	IOSocket::IOSocket(IOServer &ioServer)
		: mIOServer(ioServer)
	{
	}

	SOCKET IOSocket::Create(SocketType sockType,
		ProtocolType protocolType, DWORD flags)
	{
		SOCKET r = mSock.Create(sockType, protocolType, flags | WSA_FLAG_OVERLAPPED);
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

	void IOSocket::AsyncConnect(const NetAddress &addr, 
		ConnectIOServerData *data, notstd::HandleType proc)
	{
		data->SetHandle(proc);
		AsyncConnect(addr, data);
	}

	void IOSocket::AsyncConnect(const NetAddress &addr, ConnectIOServerData *data)
	{
		setsockopt(mSock.GetHandle(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
		mSock.Bind(notstd::NetAddress("0.0.0.0", 0));
		data->mSock = mSock.GetHandle();
		BOOL r = (*NetEnviroment::ConnectEx)(mSock.GetHandle(),
			reinterpret_cast<const sockaddr*>(&addr), sizeof(addr),
			nullptr, 0L, nullptr, static_cast<LPOVERLAPPED>(data));
		if (!r && ::GetLastError() != WSA_IO_PENDING)
			throw notstd::winSystemError();
	}

	void IOSocket::AsyncRecv(ReceiveIOServerData *data, notstd::HandleType proc)
	{
		data->SetHandle(proc);
		AsyncRecv(data);
	}

	void IOSocket::AsyncRecv(ReceiveIOServerData *data)
	{
		WSABUF buf;
		buf.buf = data->GetDataBuffer()->mBegin;
		buf.len = data->GetDataBuffer()->mCap;
		DWORD flag = 0;
		int r = ::WSARecv(mSock.GetHandle(), &buf, 1, nullptr, &flag,
			static_cast<LPOVERLAPPED>(data), nullptr);
		if (r && ::GetLastError() != WSA_IO_PENDING)
			throw notstd::winSystemError();
	}

	void IOSocket::AsyncSend(SendIOServerData *data, notstd::HandleType proc)
	{
		data->SetHandle(proc);
		AsyncSend(data);
	}

	void IOSocket::AsyncSend(SendIOServerData *data)
	{
		DWORD flag = 0;
		WSABUF buf;
		auto x = data->GetDataBuffer();
		buf.buf = x->mBegin;
		buf.len = x->mLen;
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

	void AcceptSocket::AsyncAccept(AcceptIOServerData *data, 
		IOSocket *listenSocket, notstd::HandleType proc)
	{
		data->SetHandle(proc);
		AsyncAccept(data, listenSocket);
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

	void CALLBACK IOTimer::TimeCallBack(UINT timeId, UINT msg, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
	{
		IOTimer *ioTimer = reinterpret_cast<IOTimer*>(user);
		assert(timeId == static_cast<decltype(timeId)>(ioTimer->mMMResult));
		ioTimer->mIOServer.PostUserEvent(&ioTimer->mTimeData);
	}

	void IOTimer::CloseTimer()
	{
		if (mMMResult)
		{
			::timeKillEvent(mMMResult);
			mMMResult = NULL;
		}
	}

	void IOTimer::CreateTimer(uint32_t delay, notstd::HandleType proc)
	{
		mTimeData.SetHandle(proc);
		CreateTimer(delay);
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
