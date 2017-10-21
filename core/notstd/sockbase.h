#pragma once
#include "config.h"
#include "simpleThread.h"

#ifdef PLATFORM_WINDOWS
#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif
#define	FD_SETSIZE	1024
#include <Ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/select.h>
#include <poll.h>
#define HOSTENT hostent
#endif

#if defined(PLATFORM_WINDOWS)
#include "sockbase-win32.h"
#else
#include "sockbase-posix.h"
#endif

#ifndef PLATFORM_WINDOWS
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
typedef int	SOCKET;
#endif

#ifdef PLATFORM_WINDOWS
typedef short	FAMILY_T;
typedef u_short	PORT_T;
typedef u_long	S_ADDR;
typedef int		SOCKLEN_T;
#define VALID_SOCKET(sock) (sock != INVALID_SOCKET)
#else
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef sa_family_t	FAMILY_T;
typedef in_port_t	PORT_T;
typedef in_addr_t	S_ADDR;
typedef socklen_t	SOCKLEN_T;
#define VALID_SOCKET(sock) (sock >= 0)
#endif

namespace notstd {

	enum SocketType
	{
		Stream = SOCK_STREAM,
		Dgram = SOCK_DGRAM
	};

	enum ProtocolType
	{
		Tcp = IPPROTO_TCP,
		Udp = IPPROTO_UDP
	};

	enum SocketShutdownEx
	{
#ifdef PLATFORM_WINDOWS
		BothEx = SD_BOTH,
		SendEx = SD_SEND,
		ReceiveEx = SD_RECEIVE
#else
		BothEx = SHUT_RDWR,
		SendEx = SHUT_WR,
		ReceiveEx = SHUT_RD
#endif
	};

	///////////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IPV4Address {
	public:
		S_ADDR mAddr;

	public:
		IPV4Address();
		IPV4Address(const std::string &ipAddr);
		IPV4Address(const char *ipAddr);
		IPV4Address(S_ADDR ipAddr);

		IPV4Address& Parse(const std::string &ipAddr);
		IPV4Address& Parse(const char *ipAddr);
		IPV4Address& FromDNS(const std::string &name);
		IPV4Address& FromDNS(const char *name);

		std::string ToReadableString() const;
	};

	class NOTSTD_API NetAddress : public sockaddr_in {
	public:
		NetAddress();
		NetAddress(const IPV4Address &ip, PORT_T port);
		NetAddress(const std::string &ipAddr, PORT_T port);
		NetAddress(const char *ipAddr, PORT_T port);
		NetAddress(S_ADDR ipAddr, PORT_T port);

		NetAddress& SetAddress(const IPV4Address &ip, PORT_T port);
		NetAddress& SetAddress(const char *ipAddr, PORT_T port);
		NetAddress& SetAddress(const std::string &ipAddr, PORT_T port);
		NetAddress& SetAddress(S_ADDR ipAddr, PORT_T port);

		std::string GetIPString() const;
		PORT_T GetPort() const;
	};

	///////////////////////////////////////////////////////////////////////////////

	class NOTSTD_API SocketHandle {
	private:
		SOCKET mSock;

#if !defined(PLATFORM_WINDOWS)
		static void do_brokenpipe(int);
#endif

	public:
		SocketHandle();
		~SocketHandle();

		SOCKET Create(SocketType sockType = Stream,
			ProtocolType protocolType = Tcp, DWORD flags = 0);
		int Close();
		bool Shutdown(SocketShutdownEx how);
		bool Bind(const NetAddress &netAddr);
		bool Listen(int backLog);

		bool setReuseAddr();

		bool EnableTcpKeepAlive(unsigned long keepAliveTime = 3600 * 1000,
			unsigned long keepAliveInterval = 30 * 000);
		bool DisableTcpKeepAlive();

		bool setSendBufferSize(int size);
		bool setRecvBufferSize(int size);

		bool enableNoneBlockingMode(bool enable);

		bool Connect(const char *ipAddr, PORT_T port, long timeout = 0);
		bool Connect(const NetAddress &netAddr, long timeout = 0);

		void Attach(SOCKET sock);
		SOCKET Detach();

		SOCKET GetHandle() { return mSock; }

		int ReceiveEx(char *buf, int size, DWORD *timeout);
		int SendEx(const char *buf, int size);
		bool SendHugeDataBlock(const char *buf, std::size_t size);
	};

	///////////////////////////////////////////////////////////////////////////////
	// 一些有用的工具，工具已废弃，为了一些老的项目能够编译通过才留下的

#if defined(PLATFORM_WINDOWS)
	class NOTSTD_API SocketClient
	{
	private:
		static void OnSocketClientThread(void *lParam);
		void SocketClientProc(SimpleThread *thread);

	protected:
		bool mDoNotReconnect;
		std::size_t mRecvBufferSize;
		SocketHandle mSocket;
		SimpleThread mSocketThread;

		char mIP[16];
		uint16_t mPort;

	protected:
		virtual void OnConnect();
		virtual void OnDisconnect();
		virtual bool OnReceive(const char *buf, std::size_t len);
		virtual void OnIdle();

	public:
		SocketClient();
		virtual ~SocketClient();

		bool StartSocketClient(const char *ip, uint16_t port, std::size_t recvBufSize = 8192);
		void StopSocketClient();
	};

	///////////////////////////////////////////////////////////////////////////////

	class NOTSTD_API SocketServer
	{
	protected:
		SimpleThread mServerThread;
		static void ServerThreadProc(void *lParam);
		void OnServerThread(SimpleThread *thread);

		char mIP[16];
		uint16_t mPort;

	public:
		SocketServer();
		virtual ~SocketServer();

		bool StartSockServer(const char *ip, uint16_t port);
		void StopSockServer();

	protected:
		virtual bool OnAccept(const char *localIP,
			const char *remoteIP, uint16_t remotePort, void *&userData);
		virtual void OnReceive(void *userData, const char *data, uint32_t len);
		virtual void OnClose(void *userData);
	};

	///////////////////////////////////////////////////////////////////////////////
#endif
}
