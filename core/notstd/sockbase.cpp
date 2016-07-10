#include "stdafx.h"
#include "sockmgr.h"
#include <Mswsock.h>
#include <MSTcpIP.h>
#include "stringHelper.h"
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <algorithm>
#include <regex>

#ifdef min
#undef min
#endif

namespace notstd {

	///////////////////////////////////////////////////////////////////////////////

	AcceptExProc NetEnviroment::AcceptEx = NULL;
	GetAcceptExSockaddrsProc NetEnviroment::GetAcceptExSockaddrs = NULL;
	ConnectExProc NetEnviroment::ConnectEx = NULL;

	bool NetEnviroment::Init()
	{
#ifdef PLATFORM_WINDOWS
		WSADATA wsaData;

		// 注意使用本库所需的基本的Win32平台
		WORD version = MAKEWORD(2, 2);
		int r = ::WSAStartup(version, &wsaData);
		return (r == 0) && GetExtFuncPointer();
#else
		return true;
#endif
	}

	void NetEnviroment::Term()
	{
#ifdef PLATFORM_WINDOWS
		::WSACleanup();
#endif
	}

	bool NetEnviroment::GetExtFuncPointer()
	{
#ifdef PLATFORM_WINDOWS
		SocketHandle sockHandle;

		do {
			if (!VALID_SOCKET(sockHandle.Create()))
				break;

			static struct ExtFuncEntry {
				GUID guid;
				void *outPtr;
			} efe[] = {
				{ WSAID_ACCEPTEX, &AcceptEx, },
				{ WSAID_GETACCEPTEXSOCKADDRS, &GetAcceptExSockaddrs, },
				{ WSAID_CONNECTEX, &ConnectEx, },
			};

			DWORD byteReturn;
			int r;
			for (int i = 0; i < sizeof(efe) / sizeof(efe[0]); i++)
			{
				r = ::WSAIoctl(sockHandle.GetHandle(),
					SIO_GET_EXTENSION_FUNCTION_POINTER, &efe[i].guid,
					sizeof(GUID), efe[i].outPtr, sizeof(void*),
					&byteReturn, NULL, NULL);

				if (r)
					break;
			}
			if (r)
				break;

			return true;
		} while (0);

		return false;

#endif

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////

#if !defined(PLATFORM_WINDOWS)
	void SocketHandle::do_brokenpipe(int)
	{
	}
#endif

	SocketHandle::SocketHandle()
		: mSock(INVALID_SOCKET)
	{
#if !defined(PLATFORM_WINDOWS)
		struct sigaction sa;
		sigemptyset (&sigset);
		sa.sa_flags = 0;
		sa.sa_mask = sigset;
		sa.sa_handler = &do_brokenpipe;
		sigaction (SIGPIPE, &sa, 0);
#endif
	}

	SocketHandle::~SocketHandle()
	{
		Close();
	}

	SOCKET SocketHandle::Create(SocketType sockType,
		ProtocolType protocolType, DWORD flags)
	{
		Close();

#ifdef PLATFORM_WINDOWS
		if (flags)
		{
			mSock = ::WSASocket(AF_INET, sockType, protocolType,
				NULL, 0, flags);
		}
		else
		{
			mSock = ::socket(AF_INET, sockType, protocolType);
		}
#else
		mSock = ::socket(AF_INET, sockType, protocolType);
#endif

		return mSock;
	}

	int SocketHandle::Close()
	{
		int ret = -1;
		if (VALID_SOCKET(mSock))
		{
			Shutdown(BothEx);
#ifdef PLATFORM_WINDOWS
			ret = ::closesocket(mSock);
#else
			ret = ::close(mSock);
#endif
			mSock = INVALID_SOCKET;
		}
		return ret;
	}

	bool SocketHandle::Shutdown(SocketShutdownEx how)
	{
		return !::shutdown(mSock, how);
	}

	bool SocketHandle::Bind(const NetAddress &netAddr)
	{
		return !::bind(mSock, (sockaddr*)&netAddr, sizeof(netAddr));
	}

	bool SocketHandle::Listen(int backLog)
	{
		return !::listen(mSock, backLog);
	}

	bool SocketHandle::setSendBufferSize(int size)
	{
		return (::setsockopt(mSock, SOL_SOCKET, SO_SNDBUF,
			reinterpret_cast<const char*>(&size), sizeof(int)) == 0);
	}

	bool SocketHandle::setRecvBufferSize(int size)
	{
		return (::setsockopt(mSock, SOL_SOCKET, SO_RCVBUF,
			reinterpret_cast<const char*>(&size), sizeof(int)) == 0);
	}

	bool SocketHandle::enableNoneBlockingMode(bool enable)
	{
		BOOL ret;
#ifdef PLATFORM_WINDOWS
		u_long u = (u_long)enable;
		ret = (::ioctlsocket(mSock, FIONBIO, &u) >= 0);
#else
		int d = (int)enable;
		ret = (ioctl(mSock, FIONBIO, &d) >= 0);
#endif

		return !!ret;
	}

	bool SocketHandle::EnableTcpKeepAlive(unsigned long keepAliveTime,
		unsigned long keepAliveInterval)
	{
		int r;
		//BOOL keepAlive = TRUE;
		//r = ::setsockopt(mSock, SOL_SOCKET, SO_KEEPALIVE, 
		//	reinterpret_cast<const char*>(&keepAlive), sizeof(keepAlive));
		do {
			//if (r)
			//	break;
			tcp_keepalive tcpkeepalive;
			tcpkeepalive.onoff = 1;
			// 多长时间没有数据就开始发送心跳包
			tcpkeepalive.keepalivetime = keepAliveTime;
			// 每隔多长时间发送心跳包
			tcpkeepalive.keepaliveinterval = keepAliveInterval;
			DWORD byteRet;
			r = ::WSAIoctl(mSock, SIO_KEEPALIVE_VALS, &tcpkeepalive, sizeof(tcpkeepalive),
				NULL, 0, &byteRet, NULL, NULL);
			if (r)
				break;
			return true;
		} while (0);
		return false;
	}

	bool SocketHandle::Connect(const char *ipAddr, PORT_T port, long timeout)
	{
		return Connect(NetAddress(ipAddr, port), timeout);
	}

	bool SocketHandle::Connect(const NetAddress &netAddr, long timeout)
	{
		if (timeout > 0)
		{
			bool ret;
			int r;
			fd_set wset;

			ret = false;
			if (!enableNoneBlockingMode(true))
				return ret;

			r = ::connect(mSock, reinterpret_cast<const sockaddr*>(&netAddr), sizeof(netAddr));
			if (r < 0)
			{
#if defined(PLATFORM_WINDOWS)
				if (GetLastError() != WSAEWOULDBLOCK)
					goto Out;
#else
				if (errno != EINPROGRESS)
					goto Out;
#endif

				FD_ZERO(&wset);
				FD_SET(mSock, &wset);
				timeval timeVal;
				timeVal.tv_sec = timeout / 1000;
				timeVal.tv_usec = (timeout % 1000) * 1000;
				r = ::select((int)mSock + 1, NULL, &wset, NULL, &timeVal);
				if (r <= 0)
					goto Out;
				if (!FD_ISSET(mSock, &wset))
					goto Out;
			}
			ret = true;
		Out:
			if (ret)
			{
				//TRACE("Connect succeeded. \n");
			}
			else
			{
				//TRACE("Connect fail. \n");
			}
			enableNoneBlockingMode(false);
			return ret;
		}
		else
		{
			return !::connect(mSock,
				reinterpret_cast<const sockaddr*>(&netAddr), sizeof(netAddr));
		}
	}

	void SocketHandle::Attach(SOCKET sock)
	{
		Close();
		mSock = sock;
	}

	SOCKET SocketHandle::Detach() {
		SOCKET r = mSock;
		mSock = INVALID_SOCKET;
		return r;
	}

	int SocketHandle::ReceiveEx(char *buf, int size, DWORD *timeout)
	{
		if (timeout && *timeout)
		{
			int r;
			fd_set wset;
			DWORD b = ::timeGetTime();

			FD_ZERO(&wset);
			FD_SET(mSock, &wset);
			timeval timeVal;
			timeVal.tv_sec = *timeout / 1000;
			timeVal.tv_usec = (*timeout % 1000) * 1000;

			r = ::select((int)mSock + 1, &wset, NULL, NULL, &timeVal);

			do {
				if (r <= 0)
					break;
				//if (!FD_ISSET(mSock, &wset)) {
				//	r = -1;
				//	break;
				//}
				r = ::recv(mSock, buf, size, 0);
				if (r < 0)
					break;
				if (!r) {
					r = -1;
					break;
				}
			} while (0);
			DWORD e = ::timeGetTime();
			DWORD off = e - b;
			if (off < *timeout)
				*timeout -= off;
			else
				*timeout = 0;
			return r;
		}
		else
		{
			return ::recv(mSock, buf, size, 0);
		}
	}

	int SocketHandle::SendEx(const char *buf, int size)
	{
		int sended = 0;
		while (sended < size)
		{
			int r = ::send(mSock, buf + sended, static_cast<int>(size - sended), 0);
			if (r < 0)
				return r;
			if (!r)
				return -1;
			sended += r;
		}
		return sended;
	}

	bool SocketHandle::SendHugeDataBlock(const char *buf, std::size_t size)
	{
		char sendBuf[16384];
		sendBuf;

		uint32_t leftSize = static_cast<decltype(leftSize)>(size);
		const char *sp = buf;
		while (leftSize)
		{
			int toSend = std::min(leftSize, uint32_t(sizeof(sendBuf)));
			int r = SendEx(sp, toSend);
			if (r <= 0)
				return false;
			leftSize -= toSend;
			sp += toSend;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////

	IPV4Address::IPV4Address()
		: mAddr(0)
	{
	}

	IPV4Address::IPV4Address(const std::string &ipAddr)
	{
		Parse(ipAddr);
	}

	IPV4Address::IPV4Address(const char *ipAddr)
	{
		Parse(ipAddr);
	}

	IPV4Address::IPV4Address(S_ADDR ipAddr)
	{
		mAddr = ipAddr;
	}

	IPV4Address& IPV4Address::Parse(const std::string &ipAddr)
	{
		mAddr = ::inet_addr(ipAddr.c_str());
		return *this;
	}

	IPV4Address& IPV4Address::Parse(const char *ipAddr)
	{
		mAddr = ::inet_addr(ipAddr);
		return *this;
	}

	IPV4Address& IPV4Address::FromDNS(const std::string &name)
	{
		return FromDNS(name.c_str());
	}

	IPV4Address& IPV4Address::FromDNS(const char *name)
	{
		hostent *ret = ::gethostbyname(name);
		if (ret)
			mAddr = *reinterpret_cast<UINT*>(ret->h_addr_list[0]);
		return *this;
	}

	std::string IPV4Address::ToReadableString() const
	{
		std::string r;
		UINT addr = *reinterpret_cast<const UINT*>(&mAddr);
		StringHelper::Format(r, "%d.%d.%d.%d",
			static_cast<int>((BYTE)addr),
			static_cast<int>((BYTE)(addr >> 8)),
			static_cast<int>((BYTE)(addr >> 16)),
			static_cast<int>((BYTE)(addr >> 24))
			);
		return r;
	}

	///////////////////////////////////////////////////////////////////////////////

	NetAddress::NetAddress()
	{
		memset(this, 0, sizeof(struct sockaddr_in));
	}

	NetAddress::NetAddress(const IPV4Address &ip, PORT_T port)
	{
		SetAddress(ip, port);
	}

	NetAddress::NetAddress(const std::string &ipAddr, PORT_T port)
	{
		SetAddress(ipAddr, port);
	}

	NetAddress::NetAddress(const char *ipAddr, PORT_T port)
	{
		SetAddress(ipAddr, port);
	}

	NetAddress::NetAddress(S_ADDR ipAddr, PORT_T port)
	{
		SetAddress(ipAddr, port);
	}

	NetAddress& NetAddress::SetAddress(const IPV4Address &ip, PORT_T port)
	{
		return SetAddress(ip.mAddr, port);
	}

	NetAddress& NetAddress::SetAddress(const char *ipAddr, PORT_T port)
	{
		memset(this, 0, sizeof(*this));
		sin_family = AF_INET;
		if (!ipAddr || ipAddr[0] == 0)
			sin_addr.s_addr = INADDR_ANY;
		else
			sin_addr.s_addr = ::inet_addr(ipAddr);

#ifdef PLATFORM_WINDOWS
		if (ipAddr[0] == 0)
			sin_addr.s_addr = 0;
#endif

		sin_port = htons(port);
		return *this;
	}

	NetAddress& NetAddress::SetAddress(const std::string &ipAddr, PORT_T port)
	{
		return SetAddress(ipAddr.c_str(), port);
	}

	NetAddress& NetAddress::SetAddress(S_ADDR ipAddr, PORT_T port)
	{
		memset(this, 0, sizeof(*this));
		sin_family = AF_INET;
		sin_addr.s_addr = ipAddr;
		sin_port = htons(port);
		return *this;
	}

	std::string NetAddress::GetIPString() const
	{
		std::string r;
		StringHelper::Format(r, "%d.%d.%d.%d",
			(int)(BYTE)sin_addr.s_addr,
			(int)(BYTE)(sin_addr.s_addr >> 8),
			(int)(BYTE)(sin_addr.s_addr >> 16),
			(int)(BYTE)(sin_addr.s_addr >> 24));
		return r;
	}

	PORT_T NetAddress::GetPort() const
	{
		return ntohs(sin_port);
	}

	///////////////////////////////////////////////////////////////////////////////

	SocketClient::SocketClient()
		: mDoNotReconnect(false)
	{
	}

	SocketClient::~SocketClient()
	{
		StopSocketClient();
	}

	void SocketClient::OnConnect()
	{
	}

	void SocketClient::OnDisconnect()
	{
	}

	bool SocketClient::OnReceive(const char *buf, std::size_t len)
	{
		return true;
	}

	void SocketClient::OnIdle()
	{
	}

	bool SocketClient::StartSocketClient(const char *ip, uint16_t port, std::size_t recvBufSize)
	{
		if (!recvBufSize)
			recvBufSize = 8192;
		if (recvBufSize > 64 * 1024)
			recvBufSize = 64 * 1024;

		mRecvBufferSize = recvBufSize;

		std::regex isIpAddr("\\d+.\\d+.\\d+.\\d+");
		if (std::regex_match(std::string(ip), isIpAddr))
		{
			strncpy_s(mIP, ip, 16);
			mIP[15] = 0;
		}
		else
		{
			IPV4Address ipAddr;
			ipAddr.FromDNS(ip);
			strcpy(mIP, ipAddr.ToReadableString().c_str());
		}

		mPort = port;

		return mSocketThread.startThread(&SocketClient::OnSocketClientThread, this);
	}

	void SocketClient::StopSocketClient()
	{
		mSocketThread.stopThread();
	}

	void SocketClient::OnSocketClientThread(void *lParam)
	{
		SimpleThread *thread = reinterpret_cast<SimpleThread*>(lParam);
		SocketClient *sockClient = reinterpret_cast<SocketClient*>(thread->GetUserData());
		sockClient->SocketClientProc(thread);
	}

	void SocketClient::SocketClientProc(SimpleThread *thread)
	{
		bool isConnect = false;
		char *recvBuf = new char[mRecvBufferSize];

		NetEnviroment::Init();
		while (!thread->shouldIStop())
		{
			do
			{
				if (isConnect)
					break;

				if (mDoNotReconnect)
					return;

				mSocket.Close();
				if (mSocket.Create() < 0)
				{
					::Sleep(1000);
					continue;
				}
				if (!mSocket.Connect(NetAddress(mIP, mPort), 1000))
				{
					// 有时候，错误的参数会使得Connect立即返回，不休眠会造成CPU负荷大幅度上升
					::Sleep(50);
					continue;
				}
				mSocket.EnableTcpKeepAlive(5000, 3000);
				OnConnect();
				isConnect = true;
			} while (0);

			if (isConnect)
			{
				DWORD to = 100;
				int r = mSocket.ReceiveEx(recvBuf, static_cast<int>(mRecvBufferSize), &to);
				if (r < 0)
				{
					isConnect = false;
					OnDisconnect();
					continue;
				}
				if (r > 0)
				{
					if (!OnReceive(recvBuf, static_cast<std::size_t>(r)))
						break;
				}
				else
				{
					OnIdle();
				}
			}
		}
		NetEnviroment::Term();
		delete recvBuf;
	}

	///////////////////////////////////////////////////////////////////////////////

	void SocketServer::ServerThreadProc(void *lParam)
	{
		SimpleThread *thread = reinterpret_cast<SimpleThread*>(lParam);
		SocketServer *sockServer = reinterpret_cast<SocketServer*>(thread->GetUserData());
		sockServer->OnServerThread(thread);
	}

	struct ConnPool
	{
		// 记录当前对象的个数
		std::size_t mCount;
		WSAEVENT mEvents[WSA_MAXIMUM_WAIT_EVENTS];
	};

	void SocketServer::OnServerThread(SimpleThread *thread)
	{
		typedef std::list<ConnPool> ConnPoolList;
		ConnPoolList connPoolList;

		NetEnviroment::Init();

		WSAEVENT accpetEvent = ::WSACreateEvent();
		assert(accpetEvent);

		SocketHandle sock;
		sock.Create();

		bool bindOk = false;
		do
		{
			if (sock.Bind(NetAddress(mIP, mPort)) &&
				sock.Listen(-1))
			{
				bindOk = true;
				break;
			}

			sock.Close();
		} while (!thread->shouldIStop(50));

		do {
			if (!bindOk)
				break;

			if (::WSAEventSelect(sock.GetHandle(), accpetEvent, FD_ACCEPT) != 0)
				break;

			while (!thread->shouldIStop())
			{
			}
		} while (1);

		::WSACloseEvent(accpetEvent);

		NetEnviroment::Term();
	}

	SocketServer::SocketServer()
	{
	}

	SocketServer::~SocketServer()
	{
	}

	bool SocketServer::StartSockServer(const char *ip, uint16_t port)
	{
		strncpy_s(mIP, ip, 16);
		mIP[15] = 0;

		mPort = port;

		return mServerThread.startThread(&SocketServer::ServerThreadProc, this);
	}

	void SocketServer::StopSockServer()
	{
		mServerThread.stopThread();
	}

	bool SocketServer::OnAccept(const char *localIP,
		const char *remoteIP, uint16_t remotePort, void *&userData)
	{
		return true;
	}

	void SocketServer::OnReceive(void *userData, const char *data, uint32_t len)
	{
	}

	void SocketServer::OnClose(void *userData)
	{
	}

}
