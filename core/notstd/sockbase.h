#pragma once
#include "Config.h"
#include "simpleThread.h"

extern "C"
{
	typedef BOOL(WINAPI *AcceptExProc)(
		SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
	typedef void (WINAPI *GetAcceptExSockaddrsProc)(
		PVOID, DWORD, DWORD, DWORD, LPSOCKADDR*, LPINT, LPSOCKADDR*, LPINT);
	typedef BOOL(PASCAL *ConnectExProc)(SOCKET, const struct sockaddr*, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);
}

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

class NOTSTD_API NetEnviroment {
public:
	static AcceptExProc AcceptEx;
	static GetAcceptExSockaddrsProc GetAcceptExSockaddrs;
	static ConnectExProc ConnectEx;

	static bool GetExtFuncPointer();

public:
	static bool Init();
	static void Term();
};

class NOTSTD_API SocketHandle {
private:
	SOCKET mSock;

public:
	SocketHandle();
	~SocketHandle();

	SOCKET Create(SocketType sockType = Stream,
		ProtocolType protocolType = Tcp, DWORD flags = 0);
	int Close();
	bool Shutdown(SocketShutdownEx how);
	bool Bind(const NetAddress &netAddr);
	bool Listen(int backLog);

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
// 一些有用的工具

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
