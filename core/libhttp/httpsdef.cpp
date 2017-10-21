#include "stdafx.h"
#include "httpsdef.h"
#include "notstd/notstd.h"
#include "event2/bufferevent_ssl.h"

///////////////////////////////////////////////////////////////////////////

SSLContext::SSLContext()
	: mSSLCtx(nullptr)
{
	//OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, nullptr);
	SSL_library_init();
	const SSL_METHOD* meth = SSLv23_server_method();
	mSSLCtx = SSL_CTX_new(meth);
	//SSL_CTX_set_verify(mSSLCtx, SSL_VERIFY_NONE, nullptr);
}

SSLContext::~SSLContext()
{
	if (mSSLCtx)
	{
		SSL_CTX_free(mSSLCtx);
		mSSLCtx = nullptr;
	}
}

bool SSLContext::UsePrivateKeyFile(const char *filePathName, char *password)
{
	if (password)
	{
		SSL_CTX_set_default_passwd_cb_userdata(mSSLCtx,
			reinterpret_cast<void*>(password));
	}
	if (SSL_CTX_use_PrivateKey_file(mSSLCtx, filePathName, SSL_FILETYPE_PEM) <= 0
		|| !SSL_CTX_check_private_key(mSSLCtx))
		return false;
	return true;
}

bool SSLContext::UseCertificateFile(const char *filePathName)
{
	if (SSL_CTX_use_certificate_file(mSSLCtx,
		filePathName, SSL_FILETYPE_PEM) <= 0)
		return false;
	return true;
}

bool SSLContext::SetVerify(bool verify)
{
	SSL_CTX_set_verify(mSSLCtx, SSL_VERIFY_PEER, nullptr);
	return true;
}

///////////////////////////////////////////////////////////////////////////

SSLConnection::SSLConnection()
	: mHasHandshake(false)
	, mSSLCtx(nullptr)
	, mSSL(nullptr)
{
}

SSLConnection::~SSLConnection()
{
	if (mSSL)
	{
		//SSL_free(mSSL);
		mSSL = nullptr;
	}
}

int SSLConnection::CreateSocketBuffer(event_base *eb)
{
	mBufferEvent = bufferevent_openssl_socket_new(eb, 
		mSock, mSSL, BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
	return 0;
}

void SSLConnection::SetBaseInfo(payHttpServer *server, SOCKET sock, SSL_CTX *sslCtx)
{
	Connection::SetBaseInfo(server, sock);
	mSSLCtx = sslCtx;
	mSSL = SSL_new(mSSLCtx);
}

Connection* SSLConnection::NewConnection(payHttpServer *server, SOCKET sock)
{
	auto conn = CreateRefObject<SSLConnection>();
	static_cast<SSLConnection*>(conn)->SetBaseInfo(server, sock, *server->mSSLContext);
	if (!Connection::InitializeConn(conn))
	{
		delete conn;
		return nullptr;
	}
	return conn;
}
