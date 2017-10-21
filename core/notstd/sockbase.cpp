#include "stdafx.h"
#include "sockbase.h"
#include "stringHelper.h"
#include <algorithm>
#include <chrono>

#if defined(PLATFORM_WINDOWS)
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <MSTcpIP.h>
#endif

#ifdef min
#undef min
#endif

namespace notstd {

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
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sa.sa_handler = &SocketHandle::do_brokenpipe;
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

	bool SocketHandle::setReuseAddr()
	{
		int opt = 1;
		return (::setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR,
			reinterpret_cast<char*>(&opt), sizeof(opt)) == 0);
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
#if defined(PLATFORM_WINDOWS)
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
#endif
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
			//DWORD b = ::timeGetTime();
			std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

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
			//DWORD e = ::timeGetTime();
			//DWORD off = e - b;
			std::chrono::system_clock::time_point e = std::chrono::system_clock::now();
			DWORD off = static_cast<DWORD>((e - b).count());
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
}
