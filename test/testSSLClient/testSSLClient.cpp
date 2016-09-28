#include "stdafx.h"
#include "notstd/ioServer.h"
#include <thread>
#include "notstd/xmlparserHelper.h"

class ClientProfile : public notstd::XmlElemSerializerBase
{
	DECLARE_XMLSERIAL_ELEM(ClientProfile)

private:
	notstd::uint32_tBaseTypeXmlProp mThreadCount;
	notstd::stringBaseTypeXmlProp mUrl;
	notstd::uint16_tBaseTypeXmlProp mPort;

public:
	notstd::stringBaseTypeXmlProp& GetUrl() { return mUrl; }
	notstd::uint16_tBaseTypeXmlProp& GetPort() { return mPort; }
	notstd::uint32_tBaseTypeXmlProp& GetThreadCount() { return mThreadCount; }
};

IMPLEMENT_XMLSERIAL_ELEM(ClientProfile, notstd::XmlElemSerializerBase)

BEGIN_XMLSERIAL_FLAG_TABLE(ClientProfile)
	XMLSERIAL_PROP_ENTRY_V("url",mUrl,"github.com")
	XMLSERIAL_PROP_ENTRY_V("port",mPort,"443")
	XMLSERIAL_PROP_ENTRY_V("threadCount",mThreadCount,"1")
END_XMLSERIAL_FLAG_TABLE()

notstd::SSLSocket *gTheSocket = nullptr;
notstd::SSLSendIOServerData sndData;
notstd::SSLReceiveIOServerData recvData;

static int count = 0;

void OnRecvInfo(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans)
{
	if (e)
	{
		gTheSocket->Close();
		return;
	}

	//gTheSocket->Close();
	gTheSocket->SSL_AsyncRecv(&recvData, &OnRecvInfo);
	std::cout << ++count << std::endl;
}

void OnSendInfo(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans)
{
	if (e)
	{
		gTheSocket->Close();
		return;
	}
	gTheSocket->SSL_AsyncRecv(&recvData, &OnRecvInfo);
}

void OnConnect(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans)
{
}

void OnHandShake(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
	notstd::IOServerData *data, size_t trans)
{
	std::cout << "-_-!, handshake ok\n";
	std::stringstream ss;
	ss << "GET / HTTP/1.1\r\n"
		<< "HOST: github.com\r\n"
		<< "\r\n";
	std::string toSend = ss.str();
	sndData.GetDataBuffer()->SetBufferSize(toSend.c_str(), toSend.size());
	gTheSocket->SSL_AsyncSend(&sndData, &OnSendInfo);
}

notstd::SSLContext sslContext;

int main(int argc, char **argv)
{
	std::string profPath;
	profPath.resize(MAX_PATH);
	profPath.resize(::GetModuleFileNameA(NULL, &profPath[0], MAX_PATH));
	profPath.resize(profPath.rfind('\\') + 1);
	profPath.append("testSSLClient.xml");

	ClientProfile clientProfile;
	notstd::XmlElemSerializer serial(profPath);
	serial.Load(&clientProfile);
	
	notstd::ConnectIOServerData cd;
	notstd::NetEnviroment::Init();
	notstd::IOServer ioServer;
	ioServer.CreateIOServer();

	sslContext.SetNoVerify();

	notstd::SSLSocket sock(ioServer, sslContext.GetSSLContext(), &OnHandShake);
	sock.Create();
	gTheSocket = &sock;

	notstd::IPV4Address ip;
	ip.FromDNS(clientProfile.GetUrl().mVal);
	sock.SSL_AsyncConnect(notstd::NetAddress(ip, clientProfile.GetPort().mVal), &cd, &OnConnect);

	uint32_t threadCount = clientProfile.GetThreadCount().mVal;
	if (!threadCount)
		threadCount = 1;
	std::thread *t = new std::thread[threadCount];
	for (uint32_t x = 0; x < threadCount; x++)
		t[x].swap(std::thread(std::bind(&notstd::IOServer::Run, &ioServer)));
	for (uint32_t x = 0; x < threadCount; x++)
	{
		t[x].join();
	}
	delete[] t;
    return 0;
}
