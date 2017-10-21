#include "stdafx.h"
#include "sockmgr.h"
#include <algorithm>
#include <process.h>
#include <Mswsock.h>
#include "stringHelper.h"
#include <assert.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

namespace notstd {

	///////////////////////////////////////////////////////////////////////////////

	Socket::Socket()
	{
	}

	Socket::~Socket()
	{
		Close();
	}

	void Socket::OnCustumMsg(IOCompleteData *completeData)
	{
	}

	void Socket::OnReceive(IOCompleteData *completeData)
	{
	}

	void Socket::OnSend(IOCompleteData *completeData)
	{
	}

	void Socket::OnConnect(bool connectOK)
	{
	}

	void Socket::OnAccept(const NetAddress &client,
		const NetAddress &serv)
	{
	}

	void Socket::OnClose()
	{
	}

	bool Socket::IsError()
	{
		int err = 0;
		int getAddr = sizeof(err);

		int r = ::getsockopt(GetHandle().GetHandle(), SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &getAddr);
		return !!(r || err);
	}

	bool Socket::Connect(IOCompleteData *completeData, const std::string &strIP, PORT_T port)
	{
		return Connect(completeData, NetAddress(strIP, port));
	}

	bool Socket::Connect(IOCompleteData *completeData, const NetAddress &netAddr)
	{
		completeData->mIOOperation = this;
		completeData->mOperation = IO_CONNECT;

		mSocket.Bind(NetAddress("0.0.0.0", 0));
		if (!NetEnviroment::ConnectEx)
			return false;

		if (!(*NetEnviroment::ConnectEx)(mSocket.GetHandle(),
			reinterpret_cast<const sockaddr*>(&netAddr),
			sizeof(NetAddress), NULL, 0L, NULL,
			reinterpret_cast<LPOVERLAPPED>(completeData))
			&& ::GetLastError() != ERROR_IO_PENDING)
		{
			return false;
		}

		return true;
	}

	bool Socket::Receive(IOCompleteData *completeData)
	{
		//if (completeData->mOperation != 0)
		//{
		//	assert(0);
		//}

		DWORD flags = 0;

		completeData->mIOOperation = this;
		completeData->mOperation = IO_READ;
		completeData->buf.buf = completeData->mBuffer->GetBuffer();
		completeData->buf.len = completeData->mBuffer->GetCap();

		int r = ::WSARecv(mSocket.GetHandle(), &completeData->buf, 1,
			NULL, &flags, reinterpret_cast<LPOVERLAPPED>(completeData), NULL);
		if (r == 0 || ::GetLastError() == WSA_IO_PENDING)
			return true;

		return false;
	}

	bool Socket::SendPartial(IOCompleteData *completeData)
	{
		DWORD flags = 0;

		completeData->mIOOperation = this;
		completeData->mOperation = IO_WRITE;
		completeData->buf.buf = completeData->mBuffer->GetBuffer() + completeData->hasDone;
		completeData->buf.len = completeData->tryToSend - completeData->hasDone;

		int r = ::WSASend(mSocket.GetHandle(), &completeData->buf, 1,
			NULL, flags, reinterpret_cast<LPOVERLAPPED>(completeData), NULL);
		if (r == 0 || ::GetLastError() == WSA_IO_PENDING)
			return true;

		return false;
	}

	bool Socket::Send(IOCompleteData *completeData)
	{
		//if (completeData->mOperation != 0)
		//{
		//	assert(0);
		//}

		DWORD flags = 0;

		completeData->hasDone = 0;
		completeData->tryToSend = completeData->mBuffer->GetSize();
		completeData->mIOOperation = this;
		completeData->mOperation = IO_WRITE;
		completeData->buf.buf = completeData->mBuffer->GetBuffer();
		completeData->buf.len = completeData->mBuffer->GetSize();

		int r = ::WSASend(mSocket.GetHandle(), &completeData->buf, 1,
			NULL, flags, reinterpret_cast<LPOVERLAPPED>(completeData), NULL);
		if (r == 0 || ::GetLastError() == WSA_IO_PENDING)
			return true;

		return false;
	}

	bool Socket::SendSync(IOCompleteData *completeData)
	{
		char *buf = completeData->mBuffer->GetBuffer();
		int totalSize = static_cast<int>(completeData->mBuffer->GetSize());
		return SendSync(buf, totalSize);
	}

	bool Socket::SendSync(const char *buf, std::size_t size)
	{
		int r;
		int totalSize = static_cast<int>(size);
		const char *p = buf;
		while (totalSize)
		{
			r = ::send(mSocket.GetHandle(), p, totalSize, 0);
			if (r <= 0)
				return false;
			p += r;
			totalSize -= r;
		}
		return true;
	}

	bool Socket::RecvFrom(IOCompleteData *completeData, NetAddress *addrFrom)
	{
		//DWORD flags = 0;
		//INT addrLen = sizeof(addrFrom);
		//mIODataReceive.mBuffer = pBuffer;
		//mIODataReceive.buf.buf = pBuffer->GetBuffer();
		//mIODataReceive.buf.len = pBuffer->GetCap();
		//mIODataReceive.mOperation = IO_READ;
		//int r = ::WSARecvFrom(mSocket.GetHandle(), &mIODataReceive.buf, 1, 
		//	NULL, &flags, reinterpret_cast<sockaddr*>(&addrFrom), &addrLen, 
		//	reinterpret_cast<LPOVERLAPPED>(&mIODataReceive), NULL);
		//if (r == 0 || ::GetLastError() == WSA_IO_PENDING)
		//	return true;
		return false;
	}

	bool Socket::SendTo(IOCompleteData *completeData, const NetAddress *addrTo)
	{
		//DWORD flags = 0;
		//mIODataSend.mBuffer = pBuffer;
		//mIODataSend.buf.buf	= pBuffer->GetBuffer();
		//mIODataSend.buf.len = pBuffer->GetSize();
		//mIODataSend.mOperation = IO_WRITE;
		//int r = ::WSASendTo(mSocket.GetHandle(), &mIODataSend.buf, 1, 
		//	NULL, flags, reinterpret_cast<const sockaddr*>(&addrTo), sizeof(addrTo), 
		//	reinterpret_cast<LPOVERLAPPED>(&mIODataSend), NULL);
		//if (r == 0 || ::GetLastError() == WSA_IO_PENDING)
		//	return true;
		return false;
	}

	//bool Socket::Attach(IOCompletionManager *ioManager, SOCKET sock)
	//{
	//	Close();
	//
	//	if (!ioManager)
	//		return false;
	//
	//	do {
	//		mIOManager = ioManager;
	//
	//		if (!::CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), 
	//			mIOManager->mIOCP, NULL, 0))
	//			break;
	//
	//		mSocket.Attach(sock);
	//		return true;
	//	} while (0);
	//
	//	return false;
	//}

	bool Socket::Create(IOCompletionManager *ioManager,
		SocketType nSocketType,
		ProtocolType nProtocolType, DWORD dwFlags)
	{
		Close();

		if (!ioManager->mIOCP)
			return false;

		if (!mSocket.Create(nSocketType, nProtocolType, dwFlags))
			return false;

		do {
			mIOManager = ioManager;

			if (!mIOManager->BindIOHandle(reinterpret_cast<HANDLE>(mSocket.GetHandle())))
				break;

			return true;
		} while (0);

		mSocket.Close();
		return false;
	}

	bool Socket::PostCustumMsg(IOCompleteData *completeData)
	{
		completeData->mIOOperation = this;
		completeData->mOperation = IO_CUSTUM;
		return !!::PostQueuedCompletionStatus(mIOManager->GetHandle(), 0,
			static_cast<ULONG_PTR>(0), reinterpret_cast<LPOVERLAPPED>(completeData));
	}

	bool Socket::Close()
	{
		bool r = true;
		::CancelIo(HANDLE(mSocket.GetHandle()));
		if (GetLastError() == ERROR_INVALID_HANDLE)
			r = false;
		mSocket.Close();
		return r;
	}

	SocketHandle& Socket::GetHandle()
	{
		return mSocket;
	}

	///////////////////////////////////////////////////////////////////////////////

	bool SockByAccept::Accept(IOCompleteData *completeData, Socket &sockServer)
	{
		DWORD recved;

		completeData->mIOOperation = this;
		completeData->mOperation = IO_ACCEPT;

		mServerSock = sockServer.GetHandle().GetHandle();

		if (!(*NetEnviroment::AcceptEx)(sockServer.mSocket.GetHandle(),
			mSocket.GetHandle(),
			reinterpret_cast<PVOID>(mSockAddr), 0,
			sizeof(NetAddress) + 16,
			sizeof(NetAddress) + 16,
			&recved, completeData) && ::GetLastError() != ERROR_IO_PENDING)
			return false;

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
}
