#include "stdafx.h"
#include "notstd/ioServer.h"
#include <thread>
#include <sstream>
#include "notstd/event.h"

#define THREAD_COUNT	4

notstd::AcceptSocket *gAcceptSock = nullptr;
notstd::ReceiveIOServerData recvData;
notstd::SendIOServerData sendData;

void OnSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans);

void OnRecv(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans)
{
	if (e)
	{
		std::cout << "Disconnect\n";
		return;
	}

	std::cout << "Receive ...\n";

	std::string ss;
	ss.append(reinterpret_cast<notstd::ReceiveIOServerData*>(
		data)->GetDataBuffer()->mBegin, trans);
	std::cout << ss << std::endl;

	// 随便发一些东西
	const char response[] = "HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 12\r\n"
		"\r\n"
		"Hello, world";
	sendData.GetDataBuffer()->SetBufferSize(response, strlen(response));
	gAcceptSock->AsyncSend(&sendData, std::bind(&OnSend, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

void OnSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans)
{
	if (e)
	{
		std::cout << "Disconnect\n";
		return;
	}

	gAcceptSock->AsyncRecv(&recvData, std::bind(&OnRecv, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

}

void OnAccept(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans)
{
	if (e)
	{
		std::cout << "Accept fail\n";
		return;
	}
	recvData.GetDataBuffer()->SetBufferCap(16384);
	gAcceptSock->AsyncRecv(&recvData, std::bind(&OnRecv, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

	std::cout << "OnAccept\n";
}

int main(int, char**)
{
	// 顺便连Event也一起测了
	notstd::Event eee;
	eee.CreateEvent(false);
	eee.SetEvent();
	if (eee.WaitObject(-1))
	{
		std::cout << "Ok, setEvent\n";
	}

	notstd::NetEnviroment::Init();
	notstd::IOServer ioServer;
	ioServer.CreateIOServer();

	notstd::IOSocket sock(ioServer);
	if (!sock.Create())
	{
		std::cerr << "Create sock fail\n";
		return 1;
	}

	class TestObj
	{
	private:
		notstd::SendIOServerData mSendData;
		notstd::ReceiveIOServerData mRecvData;
		notstd::IOServerQuitMessageData mQuitData;

		notstd::IOSocket &mIOSocket;
		notstd::IOServer &mIOServer;

	public:
		TestObj(notstd::IOSocket &ioSocket, notstd::IOServer &ioServer)
			: mIOServer(ioServer)
			, mIOSocket(ioSocket)
		{
		}

		void OnConnect(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans)
		{
			if (e)
			{
				std::cerr << "Connect error\n";
				return;
			}

			std::cout << "Connect ok\n";
			std::stringstream ss;
			ss << "GET / HTTP/1.1\r\n"
				<< "host: www.jd.com\r\n"
				<< "\r\n";
			std::string dd = ss.str();
			mSendData.GetDataBuffer()->SetBufferSize(dd.c_str(), dd.size());
			mIOSocket.AsyncSend(&mSendData, IOSOCKET_MEMBER_BIND(&TestObj::OnSend, this));
		}

		void OnSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans)
		{
			if (e)
			{
				std::cerr << "Send fail\n";
				return;
			}

			std::cout << "Send ok\n";
			mRecvData.GetDataBuffer()->SetBufferCap(16384);
			mIOSocket.AsyncRecv(&mRecvData, IOSOCKET_MEMBER_BIND(&TestObj::OnRecv, this));
		}

		void OnRecv(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans)
		{
			if (e)
			{
				std::cerr << "Recv fail\n";
				return;
			}

			std::cout << "recv ok, data is\n";
			std::string ss;
			ss.append(reinterpret_cast<notstd::ReceiveIOServerData*>(
				data)->GetDataBuffer()->mBegin, trans);
			//std::cout << reinterpret_cast<notstd::ReceiveIOServerData*>(
			//	data)->GetDataBuffer()->mBegin << " \n";
			std::cout << ss << std::endl;

			std::cout << "1111\n";
			mRecvData.GetDataBuffer()->SetBufferCap(16384);
			std::cout << "2222\n";
			mIOSocket.AsyncRecv(&mRecvData, IOSOCKET_MEMBER_BIND(&TestObj::OnRecv, this));
			std::cout << "3333\n";
			//for (int i = 0; i < THREAD_COUNT; i++)
			//	ioServer->PostQuit(&mQuitData);
		}
	} mTestObj(sock, ioServer);

	notstd::ConnectIOServerData connData;
	notstd::IPV4Address addrForTarget;
	addrForTarget.FromDNS("www.jd.com");
	//sock.AsyncConnect(notstd::NetAddress(addrForTarget, 80), &connData, 
	//	IOSOCKET_MEMBER_BIND(&TestObj::OnConnect, &mTestObj));

	notstd::AcceptIOServerData acceptData;
	notstd::AcceptSocket as(ioServer);
	notstd::IOSocket listenSock(ioServer);

	do {
		if (as.Create() < 0)
		{
			std::cerr << "Create as fail\n";
			break;
		}
		if (listenSock.Create() < 0)
		{
			std::cerr << "Create listensock fail\n";
			break;
		}
		listenSock.GetHandle().setReuseAddr();
		if (!listenSock.GetHandle().Bind(notstd::NetAddress("0.0.0.0", 8080)))
		{
			std::cerr << "Bind listen sock fail\n";
			break;
		}
		if (!listenSock.GetHandle().Listen(-1))
		{
			std::cerr << "Listen on socket fail\n";
			break;
		}
		gAcceptSock = &as;
		as.AsyncAccept(&acceptData, &listenSock, std::bind(&OnAccept, std::placeholders::_1,
			std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	} while (0);

	std::thread thread[THREAD_COUNT];
	for (int i = 0; i < THREAD_COUNT; i++)
		thread[i] = std::move(std::thread(std::bind(&notstd::IOServer::Run, &ioServer)));

	for (int i = 0; i < THREAD_COUNT; i++)
		thread[i].join();

    return 0;
}
