#pragma once

#include "config.h"

// 由于OnConnect函数引用了一些SOCKET类型，所以必须包含这些头文件
// 在linux和win32中进行socket编程所需的头文件和库
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

#ifndef PLATFORM_WINDOWS
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
typedef int				SOCKET;
#endif

#ifdef PLATFORM_WINDOWS
typedef short		FAMILY_T;
typedef u_short	PORT_T;
typedef u_long	S_ADDR;
typedef int			SOCKLEN_T;
#define VALID_SOCKET(sock) (sock != INVALID_SOCKET)
#elif
typedef sa_family_t	FAMILY_T;
typedef in_port_t		PORT_T;
typedef in_addr_t		S_ADDR;
typedef socklen_t		SOCKLEN_T;
#define VALID_SOCKET(sock) (sock >= 0)
#endif

enum {
	IO_NOUSE,
	IO_TERMINATE,
	IO_READ,
	IO_WRITE,
	IO_READFROM,
	IO_WRITETO,
	IO_CONNECT,
	IO_ACCEPT,
	IO_CUSTUM,
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

class NOTSTD_API Buffer
{
public:
	virtual uint32_t GetCap() const = 0;
	virtual uint32_t GetSize() const = 0;
	virtual char* GetBuffer() = 0;
	virtual void AppendData(const char *str, std::size_t s) = 0;
	virtual void SetLength(std::size_t s) = 0;
	virtual void SetCap(std::size_t s) = 0;
};

class IOOperationBase;

struct NOTSTD_API IOCompleteData : public OVERLAPPED
{
	DWORD mOperation;
	IOOperationBase *mIOOperation;
	Buffer *mBuffer;
	WSABUF buf;
	UINT hasDone;
	UINT tryToSend;

	IOCompleteData();
};

class NOTSTD_API IOCompletionManager
{
	friend class Socket;
	friend class Serial;

private:
	HANDLE mIOCP;

	UINT mThreadCount;
	HANDLE *mIOCompletionThreads;

private:
	static UINT WINAPI CompletionThread(LPVOID lpParam);
	void CompletionProc();
	void PostTerminateSignal();

public:
	IOCompletionManager();
	virtual ~IOCompletionManager();

	virtual bool StartManager(DWORD minThread = 0);
	virtual void StopManager();
	virtual void Join();
	virtual bool BindIOHandle(HANDLE handle);

	HANDLE GetHandle() { return mIOCP; }
};

class NOTSTD_API IOOperationBase {
	friend class IOCompletionManager;

protected:
	virtual void OnCustumMsg(IOCompleteData *completeData) = 0;
	virtual void OnReceive(IOCompleteData *completeData) = 0;
	virtual void OnSend(IOCompleteData *completeData) = 0;
	virtual void OnConnect(bool connectOK) = 0;
	virtual void OnAccept(const NetAddress &client, const NetAddress &serv) = 0;
	virtual void OnClose() = 0;
	virtual bool IsError() = 0;

	virtual bool SendPartial(IOCompleteData *completeData) = 0;

public:
	virtual ~IOOperationBase();
};

class NOTSTD_API AdvIOBuffer : public Buffer {
protected:
	char *mBuf;
	// 对外报告的容量
	uint32_t mReportCap;
	// 实际的容量
	uint32_t mRealCap;
	// 数据量
	uint32_t mSize;

protected:
	bool Malloc(std::size_t len);

public:
	virtual uint32_t GetCap() const;
	virtual uint32_t GetSize() const;
	virtual char* GetBuffer();
	virtual void AppendData(const char *str, std::size_t len);
	virtual void SetLength(std::size_t s);
	virtual void SetCap(std::size_t s);

	AdvIOBuffer(UINT defaultSize = 4096);
	virtual ~AdvIOBuffer();
};

struct NOTSTD_API DefaultCompleteData : public IOCompleteData
{
	AdvIOBuffer mIOBuffer;

public:
	DefaultCompleteData();
};
