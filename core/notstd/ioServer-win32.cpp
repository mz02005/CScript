#include "stdafx.h"
#include "ioServer.h"
#include <Mswsock.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

namespace notstd {


	///////////////////////////////////////////////////////////////////////////

	AcceptIOServerData::AcceptIOServerData()
		: SocketIOServerData(IOS_ACCEPT)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	INT OnCloseTimerException(DWORD code)
	{
		INT r = EXCEPTION_CONTINUE_SEARCH;
		if (code == EXCEPTION_INVALID_HANDLE
			|| code == STATUS_INVALID_PARAMETER)
			return EXCEPTION_EXECUTE_HANDLER;
		return r;
	}

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

	notstdException::~notstdException()
	{
		if (mBuf)
			delete[] mBuf;
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
		//OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, nullptr);
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

	void IOServer::PostQuit(IOServerQuitMessageData *quitData)
	{
		::PostQueuedCompletionStatus(mIOServer, 0, NULL,
			static_cast<LPOVERLAPPED>(quitData));
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

			case IOServerData::IOS_TIMER:
				do {
					ioData->OnFunc(this, ec, ioData, transferred);
					TimerData *td = static_cast<TimerData*>(ioData);
					td->GetIOTimer()->AfterOnTimer();
				} while (0);
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
		for (; RunOneStep();)
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

	bool IOSocket::AsyncConnect(const NetAddress &addr,
		ConnectIOServerData *data, notstd::HandleType proc,
		const NetAddress &local)
	{
		data->SetHandle(proc);
		return AsyncConnect(addr, data);
	}

	bool IOSocket::AsyncConnect(const NetAddress &addr, ConnectIOServerData *data,
		const NetAddress &local)
	{
		bool r = true;
		setsockopt(mSock.GetHandle(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
		mSock.Bind(local);
		data->mSock = mSock.GetHandle();
		__try {
			BOOL r = (*NetEnviroment::ConnectEx)(mSock.GetHandle(),
				reinterpret_cast<const sockaddr*>(&addr), sizeof(addr),
				nullptr, 0L, nullptr, static_cast<LPOVERLAPPED>(data));
			if (!r && ::GetLastError() != WSA_IO_PENDING)
				r = false;
		}
		__except (OnCloseTimerException(GetExceptionCode()))
		{
			r = false;
		}

		return r;
	}

	bool IOSocket::AsyncRecv(ReceiveIOServerData *data, notstd::HandleType proc)
	{
		data->SetHandle(proc);
		return AsyncRecv(data);
	}

	bool IOSocket::AsyncRecv(ReceiveIOServerData *data)
	{
		bool r = true;
		WSABUF buf;
		buf.buf = data->GetDataBuffer()->mBegin;
		buf.len = data->GetDataBuffer()->mCap;
		DWORD flag = 0;

		__try
		{
			int r = ::WSARecv(mSock.GetHandle(), &buf, 1, nullptr, &flag,
				static_cast<LPOVERLAPPED>(data), nullptr);
			if (!r && ::GetLastError() != WSA_IO_PENDING)
				r = false;
		}
		__except (OnCloseTimerException(GetExceptionCode()))
		{
			r = false;
		}

		return r;
	}

	bool IOSocket::AsyncSend(SendIOServerData *data, notstd::HandleType proc)
	{
		data->SetHandle(proc);
		return AsyncSend(data);
	}

	bool IOSocket::AsyncSend(SendIOServerData *data)
	{
		bool r = true;
		DWORD flag = 0;
		WSABUF buf;
		auto x = data->GetDataBuffer();
		buf.buf = x->mBegin;
		buf.len = x->mLen;

		__try
		{
			int r = ::WSASend(mSock.GetHandle(), &buf, 1, nullptr, flag,
				static_cast<LPWSAOVERLAPPED>(data), nullptr);
			if (!r && ::GetLastError() != WSA_IO_PENDING)
				r = false;
		}
		__except (OnCloseTimerException(GetExceptionCode()))
		{
			r = false;
		}

		return r;
	}

	///////////////////////////////////////////////////////////////////////////

	AcceptSocket::AcceptSocket(IOServer &ioServer)
		: IOSocket(ioServer)
	{
	}

	bool AcceptSocket::AsyncAccept(AcceptIOServerData *data,
		IOSocket *listenSocket, notstd::HandleType proc)
	{
		data->SetHandle(proc);
		return AsyncAccept(data, listenSocket);
	}

	bool AcceptSocket::AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket)
	{
		bool r = true;
		DWORD recved;
		data->mSock = mSock.GetHandle();
		data->mServerSocket = listenSocket->GetHandle().GetHandle();

		__try
		{
			if (!(*NetEnviroment::AcceptEx)(listenSocket->GetHandle().GetHandle(),
				mSock.GetHandle(),
				reinterpret_cast<PVOID>(data->mSockAddr), 0,
				sizeof(NetAddress) + 16,
				sizeof(NetAddress) + 16,
				&recved, static_cast<LPOVERLAPPED>(data)) && ::GetLastError() != ERROR_IO_PENDING)
				r = false;
		}
		__except (OnCloseTimerException(GetExceptionCode()))
		{
			r = false;
		}

		return r;
	}

	///////////////////////////////////////////////////////////////////////////

	TimerData::TimerData()
		: IOServerData(IOS_TIMER)
	{
	}

	void TimerData::OnFunc(IOServer *ioServer, const IOErrorCode &e,
		IOServerData *data, size_t trans)
	{
		if (mHandleFunc)
			mHandleFunc(ioServer, true, data, trans);
	}

	IOTimer::IOTimer(IOServer &ioServer)
		: mIOServer(ioServer)
		, mTimeQueueTimer(NULL)
		, mDueTime(1000)
		, mPeriod(0)
		, mTimerData(nullptr)
	{
	}

	IOTimer::~IOTimer()
	{
		CloseTimer();
	}

	bool IOTimer::BeforeOnTimer()
	{
		return true;
	}

	void IOTimer::AfterOnTimer()
	{
	}

	void CALLBACK IOTimer::TimeCallBack(PVOID parameter, BOOLEAN timerOrWaitFired)
	{
		IOTimer *ioTimer = reinterpret_cast<IOTimer*>(parameter);
		if (!ioTimer->BeforeOnTimer())
			return;
		ioTimer->mIOServer.PostUserEvent(ioTimer->mTimerData);
		// 对于只运行一次的定时器，还需要清理定时器的数据
		if (!ioTimer->mPeriod)
			ioTimer->CloseTimer(false);
	}

	void IOTimer::CloseTimer(bool waitOver)
	{
		if (mTimeQueueTimer)
		{
			__try
			{
				::DeleteTimerQueueTimer(NULL, mTimeQueueTimer,
					waitOver ? INVALID_HANDLE_VALUE : nullptr);
			}
			__except (OnCloseTimerException(GetExceptionCode()))
			{
			}
			mTimeQueueTimer = NULL;
		}
	}

	bool IOTimer::CreateTimer(uint32_t dueTime, uint32_t period,
		TimerData *timeData, notstd::HandleType proc)
	{
		mTimerData = timeData;
		mTimerData->SetHandle(proc);
		return CreateTimer(dueTime, period);
	}

	bool IOTimer::CreateTimer(uint32_t dueTime, uint32_t period)
	{
		CloseTimer(false);
		assert(mTimerData);
		mDueTime = dueTime;
		mPeriod = period;
		mTimerData->mIOTimer = this;
		BOOL r = ::CreateTimerQueueTimer(&mTimeQueueTimer, NULL,
			&IOTimer::TimeCallBack, reinterpret_cast<PVOID>(this), mDueTime,
			mPeriod, WT_EXECUTEINTIMERTHREAD);
		return r && mTimeQueueTimer;
	}

	///////////////////////////////////////////////////////////////////////////
}
