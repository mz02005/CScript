#include "stdafx.h"
#include "notstd/ioServer.h"
#include <functional>
#include <thread>

const static char *hostName = "www.cnbeta.com";
const static char *url = "/";

class AAA
{
public:
	void OnSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		if (e)
			return;
		notstd::SendIOServerData *sd = static_cast<notstd::SendIOServerData*>(data);
		if (trans == sd->GetBufferAndSize().second)
			// 命令发送完了，开始接收
			mIOSocket.AsyncRecv(&mRecvData, IOSOCKET_MEMBER_BIND(&AAA::OnReceive1, this));
		else {
			// 没有发送完成，继续发送
			sd->mBegin += trans;
			sd->mLen -= trans;
			mIOSocket.AsyncSend(static_cast<notstd::SendIOServerData*>(data));
		}
	}

	void OnConnect(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		if (e)
		{
			printf("Connect fail %s\n", e.what().c_str());
			mIOSocket.Close();
			return;
		}

		printf("Connect succeeded\n");

		// 暂时先不做更多的数据读取了
		mIOSocket.Close();
		return;

		std::string s;
		s.append("GET ").append(url).append(" HTTP/1.1\r\n");
		s += "Connection: close\r\n"
			"Host: ";
		s += hostName;
		s += "\r\n\r\n";
		mSendData.SetSendBuffer(s.c_str(), s.size());
		mIOSocket.AsyncSend(&mSendData, IOSOCKET_MEMBER_BIND(&AAA::OnSend, this));
	}

	void OnReceive(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		if (e)
		{
			if (trans == 0)
			{
				printf("\nDisconnect\n");
				return;
			}
			printf("Connect fail %s\n", e.what().c_str());
			mIOSocket.Close();
			return;
		}

		std::string s;
		notstd::ReceiveIOServerData *rd = static_cast<notstd::ReceiveIOServerData*>(data);
		s.append(rd->mBegin, trans);
		std::cout << s;
		mIOSocket.AsyncRecv(&mRecvData);
	}

	void OnReceive1(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		if (e)
			return;
		std::string s;
		notstd::ReceiveIOServerData *rd = static_cast<notstd::ReceiveIOServerData*>(data);
		s.append(rd->mBegin, trans);
		std::cout << s;
		mAcceptSocket.AsyncRecv(&mRecvData1);
	}
	
	void OnAccept(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		if (e)
			return;

		mAcceptSocket.AsyncRecv(&mRecvData1, IOSOCKET_MEMBER_BIND(&AAA::OnReceive1, this));
	}

	void OnTimer(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		static int xx = 0;
		if (++xx == 10)
		{
			//ioServer->PostQuit();
		}
		std::cout << "HAHA " << xx << std::endl;
	}

private:
	notstd::IOSocket &mIOSocket;
	notstd::ReceiveIOServerData mRecvData;
	notstd::SendIOServerData mSendData;

	notstd::ReceiveIOServerData mRecvData1;
	notstd::AcceptSocket &mAcceptSocket;
	notstd::IOSocket &mListenSocket;

public:
	AAA(notstd::IOSocket &ioSocket, notstd::IOSocket &listenSocket, notstd::AcceptSocket &acceptSock)
		: mIOSocket(ioSocket)
		, mListenSocket(listenSocket)
		, mAcceptSocket(acceptSock)
	{
	}
};

class TestConnection
{
public:
	notstd::ConnectIOServerData mCd;
	notstd::SendIOServerData mSd;
	notstd::IOServer mIOServer;
	notstd::IOSocket mConnSocket;

public:
	TestConnection(notstd::IOServer &ioServer)
		: mIOServer(ioServer)
		, mConnSocket(ioServer)
	{
		mConnSocket.Create();
	}

	notstd::IOSocket& GetConnSocket() { return mConnSocket; }

	void OnConnect(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		if (e) {
			std::cerr << "Connect fail\n";
			return;
		}
	}

	void OnSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, notstd::IOServerData *data, size_t trans)
	{
		if (e) {
			std::cerr << "Send fail\n";
			return;
		}

		notstd::IOServerDataWithCache *d = static_cast<notstd::IOServerDataWithCache*>(data);
		d->mLen -= trans;
		if (d->mLen)
		{
			d->mBegin += trans;
			mConnSocket.AsyncSend(&mSd);
		}
		else
		{
			std::cout << "send over\n";
		}
	}

	void DoConnect()
	{
		mConnSocket.AsyncConnect(notstd::NetAddress("192.168.192.228", 3015), &mCd,
			IOSOCKET_MEMBER_BIND(&TestConnection::OnConnect, this));
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	notstd::NetEnviroment::Init();
	notstd::IOServer ioServer;
	ioServer.CreateIOServer();
	notstd::IOSocket ioSocket(ioServer);

	notstd::IOSocket sockListen(ioServer);
	sockListen.Create();
	if (!sockListen.GetHandle().Bind(notstd::NetAddress("0.0.0.0", 8080)))
	{
		printf("Bind local fail\n");
		return 1;
	}
	if (!sockListen.GetHandle().Listen(SOMAXCONN))
	{
		printf("Listen fail\n");
		return 2;
	}

	notstd::AcceptSocket as(ioServer);
	notstd::AcceptIOServerData aiod;

	AAA aaa(ioSocket, sockListen, as);
	notstd::IOTimer theTimer(ioServer);

	theTimer.CreateTimer(1000, IOSOCKET_MEMBER_BIND(&AAA::OnTimer, &aaa));

	as.Create();
	as.AsyncAccept(&aiod, &sockListen,
		IOSOCKET_MEMBER_BIND(&AAA::OnAccept, &aaa));

	TestConnection tc(ioServer);
	tc.DoConnect();

	std::thread x(std::bind(&notstd::IOServer::Run, &ioServer));

	x.join();

	return 0;
}
