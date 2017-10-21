#include "stdafx.h"
#include "sockbase.h"
#include <Mswsock.h>
#include <regex>

namespace notstd {

	///////////////////////////////////////////////////////////////////////////////

	AcceptExProc NetEnviroment::AcceptEx = NULL;
	GetAcceptExSockaddrsProc NetEnviroment::GetAcceptExSockaddrs = NULL;
	ConnectExProc NetEnviroment::ConnectEx = NULL;

	bool NetEnviroment::Init()
	{
		WSADATA wsaData;

		// 注意使用本库所需的基本的Win32平台
		WORD version = MAKEWORD(2, 2);
		int r = ::WSAStartup(version, &wsaData);
		return (r == 0) && GetExtFuncPointer();
	}

	void NetEnviroment::Term()
	{
		::WSACleanup();
	}

	bool NetEnviroment::GetExtFuncPointer()
	{
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
