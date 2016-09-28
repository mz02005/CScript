#include "stdafx.h"
#include "notstd/ioServer.h"
#include <thread>

#include "certDef.h"

static const char *cont = R"(
<html><head><title>Congratulations</title></head>
<body><h1>
Welcome to mz's https site.</h1>
</body></html>
)";

//static int ssl_get_error(SSL *ssl, int result)
//{
//	int error = SSL_get_error(ssl, result);
//	if (SSL_ERROR_NONE != error)
//	{

//		char message[512] = { 0 };
//		int error_log = error;
//		while (SSL_ERROR_NONE != error_log)
//		{
//
//			ERR_error_string_n(error_log, message, _countof(message));
//			printf("%s\n", message);
//			error_log = ERR_get_error();
//		}
//	}
//	return error;
//}

notstd::SSLContext gSSLContext;

void usecert()
{
	gSSLContext.UseCertificateFile(PROJECT_DIR "server.crt");
	gSSLContext.UsePrivateKeyFile(PROJECT_DIR "private.key", "000000");
}

class SSLConnectionNT
{
	notstd::IOServer &mIOServer;
	notstd::SSLSocket mSSLSocket;

	notstd::IOSocket &mListenSocket;

	notstd::AcceptIOServerData mAcceptData;

	notstd::SSLReceiveIOServerData mRecvData;
	notstd::SSLSendIOServerData mSendData;

private:
	void OnRecv(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		if (e)
		{
			if (!trans)
			{
				std::cout << "Disconnect\n";
			}
			mSSLSocket.Close();
			return;
		}

		std::cout << "Receive the request from browser\n";

		std::stringstream ss;
		ss << "HTTP/1.1 200 OK\r\n"
			<< "Content-Length: " << (ULONGLONG)strlen(cont) << "\r\n\r\n";
		ss << cont;
		std::string toSend = ss.str();
		mSendData.GetDataBuffer()->SetBufferSize(toSend.c_str(), toSend.size());
		mSSLSocket.SSL_AsyncSend(&mSendData,
			IOSOCKET_MEMBER_BIND(&SSLConnectionNT::OnSend, this));
	}

	void OnSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		mSSLSocket.SSL_AsyncRecv(&mRecvData);
		return;
	}

	void OnAccept(notstd::IOServer *ioServer, const notstd::IOErrorCode &e, 
		notstd::IOServerData *data, size_t trans)
	{
		SSLConnectionNT::CreateConnection(mIOServer, mListenSocket);
	}

	void OnHandShake(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		if (e)
		{
			mSSLSocket.Close();
			return;
		}
		std::cout << "Handshake ok\n";
		mSSLSocket.SSL_AsyncRecv(&mRecvData,
			IOSOCKET_MEMBER_BIND(&SSLConnectionNT::OnRecv, this));
	}

public:
	SSLConnectionNT(notstd::IOServer &ioServer, notstd::IOSocket &ioSocket)
		: mIOServer(ioServer)
		, mSSLSocket(ioServer, gSSLContext.GetSSLContext(), 
			IOSOCKET_MEMBER_BIND(&SSLConnectionNT::OnHandShake, this))
		, mListenSocket(ioSocket)
	{
		mSSLSocket.Create();
	}

	static SSLConnectionNT* CreateConnection(notstd::IOServer &ioServer, 
		notstd::IOSocket &listenSocket)
	{
		SSLConnectionNT *newConn = new SSLConnectionNT(ioServer, listenSocket);
		newConn->mSSLSocket.SSL_AsyncAccept(&newConn->mAcceptData, &listenSocket,
			IOSOCKET_MEMBER_BIND(&SSLConnectionNT::OnAccept, newConn));
		return newConn;
	}
};

int main(int argc, char **argv)
{
	notstd::NetEnviroment::Init();
	notstd::IOServer ioServer;
	ioServer.CreateIOServer();

	usecert();

	notstd::AcceptSocket asBBB(ioServer);
	notstd::IOSocket listenSocketForBBB(ioServer);
	
	listenSocketForBBB.Create();
	if (!listenSocketForBBB.GetHandle().Bind(notstd::NetAddress("0.0.0.0", 8081)))
	{
		printf("Bind bbb fail\n");
		return 2;
	}
	if (!listenSocketForBBB.GetHandle().Listen(SOMAXCONN))
	{
		printf("listen bbb fail\n");
		return 3;
	}

	//SSLConnection::CreateSSLConnection(ioServer, listenSocketForBBB);
	SSLConnectionNT::CreateConnection(ioServer, listenSocketForBBB);

	std::thread x(std::bind(&notstd::IOServer::Run, &ioServer));

	x.join();

	return 0;
}
