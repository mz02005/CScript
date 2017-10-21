#include "stdafx.h"
#include "ioServer.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

inline void OutputDebugStringHelper(const char *format, ...)
{
	char buf[1024];
	char *tempBuf = nullptr;

	va_list valist;
	va_start(valist, format);
	int r = vsnprintf(buf, sizeof(buf), format, valist);
	if (r >= sizeof(buf))
	{
		tempBuf = reinterpret_cast<char*>(malloc(r));
		vsnprintf(tempBuf, r, format, valist);
	}
	va_end(valist);

	OutputDebugStringA(tempBuf ? tempBuf : buf);
	if (tempBuf)
		free(tempBuf);
}

namespace notstd {

	void SSLSocket::OnSSLReceive(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		if (e)
		{
			if (!trans)
			{
				std::cout << "Disconnect\n";
			}
			mSock.Close();
			return;
		}

		auto buffer = mRecvDataForSSLHandShake.GetDataBuffer();
		int r = BIO_write(mBIO[RECV], buffer->mBegin, trans);
		int ssl_error = SSL_get_error(mSSL, r);

		if (SSL_ERROR_NONE == ssl_error)
		{
			r = SSL_read(mSSL, mRecvBuffer, sizeof(mRecvBuffer));
			OutputDebugStringHelper("SSL_read return %d(@%s:%d)\n", r, __FILE__, __LINE__);
			ssl_error = SSL_get_error(mSSL, r);
			switch (ssl_error)
			{
			case SSL_ERROR_SSL:
				std::cout << "SSL error\n";
				mSock.Close();
				return;
			}

			// 看看是不是有应答可以发送
			std::string toSendData;
			do
			{
				int toSend = BIO_read(mBIO[SEND], mSendBuffer, sizeof(mSendBuffer));
				if (toSend > 0)
					toSendData.append(mSendBuffer, toSend);
			} while (BIO_pending(mBIO[SEND]));
			if (toSendData.size())
			{
				mSendDataForSSLHandShake.GetDataBuffer()->SetBufferSize(toSendData.c_str(), toSendData.size());
				AsyncSend(&mSendDataForSSLHandShake, IOSOCKET_MEMBER_BIND(&SSLSocket::OnSSLSend, this));
			}
			else {
				// 看看是不是已经完成了握手
				if (SSL_is_init_finished(mSSL))
				{
					OutputDebugStringHelper("Handshake ok @(%s:%d)\n", __FILE__, __LINE__);
					mHasHandShake = true;
					if (mUserOnHandShake)
						mUserOnHandShake(ioServer, e, data, trans);
					return;
				}

				OutputDebugStringHelper("BIO read return nothing, so try to receive more data(@%s:%d)\n", __FILE__, __LINE__);
				AsyncRecv(&mRecvDataForSSLHandShake);
			}
		}
		else if (SSL_ERROR_SSL == ssl_error)
		{
			OutputDebugStringHelper("BIO_write return %d(@%s:%d), error occurred\n", r, __FILE__, __LINE__);
			mSock.Close();
			return;
		}
		else
			AsyncRecv(&mRecvDataForSSLHandShake);
	}

	void SSLSocket::OnSSLSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		if (e)
		{
			if (!trans)
				std::cout << "Disconnected.\n";
			mSock.Close();
			return;
		}

		if (!mHasHandShake)
		{
			if (SSL_is_init_finished(mSSL))
			{
				OutputDebugStringHelper("Handshake ok\n");
				mHasHandShake = true;
				if (mUserOnHandShake)
					mUserOnHandShake(ioServer, e, data, trans);
			}
			else
			{
				OutputDebugStringHelper("Handshake not complete, try to receive more data\n");
				AsyncRecv(&mRecvDataForSSLHandShake, IOSOCKET_MEMBER_BIND(&SSLSocket::OnSSLReceive, this));
			}
		}
		else
		{
			OutputDebugStringHelper("Strange error occurred\n");
			assert(false);
		}
	}

	void SSLSocket::OnSSLConnect(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		if (mUserOnAcceptOrUserOnConnect)
			(mUserOnAcceptOrUserOnConnect)(ioServer, e, data, trans);

		if (e)
			return;

		// 开始SSL握手的过程
		SSL_set_connect_state(mSSL);

		int toRead = SSL_read(mSSL, mRecvBuffer, sizeof(mRecvBuffer));
		int sslerror = SSL_get_error(mSSL, toRead);
		
		// 看看是不是有应答可以发送
		std::string toSendData;
		while (BIO_pending(mBIO[SEND]))
		{
			int toSend = BIO_read(mBIO[SEND], mSendBuffer, sizeof(mSendBuffer));
			if (toSend > 0)
				toSendData.append(mSendBuffer, toSend);
		}
		if (toSendData.size())
		{
			mSendDataForSSLHandShake.GetDataBuffer()->SetBufferSize(toSendData.c_str(), toSendData.size());
			AsyncSend(&mSendDataForSSLHandShake, IOSOCKET_MEMBER_BIND(&SSLSocket::OnSSLSend, this));
		}
	}

	void SSLSocket::OnSSLAccept(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		// 先调用用户的OnAccept，让用户有机会可以执行新的异步Accept
		if (mUserOnAcceptOrUserOnConnect)
			(mUserOnAcceptOrUserOnConnect)(ioServer, e, data, trans);

		SSL_set_accept_state(mSSL);
		// 开始SSL握手的过程
		AsyncRecv(&mRecvDataForSSLHandShake,
			IOSOCKET_MEMBER_BIND(&SSLSocket::OnSSLReceive, this));
	}

	void SSLSocket::OnSendAfterHandShake(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		SSLSendIOServerData *sslSendData = static_cast<SSLSendIOServerData*>(data);

		if (e)
		{
			sslSendData->mUserCallback(ioServer, e, data, trans);
			return;
		}

		sslSendData->mUserCallback(ioServer, e, data, trans);
	}

	void SSLSocket::OnRecvAfterHandShake(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
		notstd::IOServerData *data, size_t trans)
	{
		SSLReceiveIOServerData *sslRecvData = static_cast<SSLReceiveIOServerData*>(data);

		if (e)
		{
			sslRecvData->mUserCallback(ioServer, e, data, trans);
			return;
		}

		// 需要解密
		std::string userData;
		if (!ReadFromSSLStream(sslRecvData->GetDataBuffer()->mBegin, trans, userData))
		{
			sslRecvData->mUserCallback(ioServer, false, data, trans);
			return;
		}
		if (userData.size())
		{
			sslRecvData->GetDataBuffer()->SetBufferSize(userData.c_str(), userData.size());
			sslRecvData->mUserCallback(ioServer, e, sslRecvData, userData.size());
		}
	}

	bool SSLSocket::ReadFromSSLStream(const char *o, size_t s, std::string &data)
	{
		int r, sslerr;
		data.clear();
		r = BIO_write(mBIO[RECV], o, s);
		sslerr = SSL_get_error(mSSL, r);
		if (sslerr == SSL_ERROR_SSL)
			return false;
		if (sslerr != SSL_ERROR_NONE)
			return true;
		do {
			r = SSL_read(mSSL, mRecvBuffer, sizeof(mRecvBuffer));
			sslerr = SSL_get_error(mSSL, r);
			if (sslerr == SSL_ERROR_SSL)
				return false;
			if (r > 0)
				data.append(mRecvBuffer, r);
		} while (SSL_pending(mSSL));
		return true;
	}

	bool SSLSocket::WriteToSSLStream(const char *toWrite, size_t s, std::string &data)
	{
		int r, sslerr;
		data.clear();
		r = SSL_write(mSSL, toWrite, s);
		sslerr = SSL_get_error(mSSL, r);
		if (sslerr == SSL_ERROR_SSL)
			return false;
		if (sslerr != 0)
			return true;
		do {
			r = BIO_read(mBIO[SEND], mSendBuffer, sizeof(mSendBuffer));
			sslerr = SSL_get_error(mSSL, r);
			if (sslerr == SSL_ERROR_SSL)
				return false;
			if (r > 0)
				data.append(mSendBuffer, r);
		} while (BIO_pending(mBIO[SEND]));
		return true;
	}

	SSLSocket::SSLSocket(notstd::IOServer &ioServer, SSL_CTX *sslctx, 
		notstd::HandleType onHandShake)
		: notstd::AcceptSocket(ioServer)
		, mSSLContext(sslctx)
		, mUserOnHandShake(onHandShake)
		, mHasHandShake(false)
		, mMoreRead(false)
	{
		if (!mUserOnHandShake)
		{
			throw std::exception("onHandShake must not be zero");
		}

		mSSL = SSL_new(mSSLContext);
		mBIO[0] = BIO_new(BIO_s_mem());
		mBIO[1] = BIO_new(BIO_s_mem());
		SSL_set_bio(mSSL, mBIO[RECV], mBIO[SEND]);
	}

	SSLSocket::~SSLSocket()
	{
		if (mSSL)
		{
			if (mBIO[0])
				BIO_free(mBIO[0]);
			if (mBIO[1])
				BIO_free(mBIO[1]);
			mBIO[0] = mBIO[1] = nullptr;
			SSL_free(mSSL);
			mSSL = nullptr;
		}
	}

	void SSLSocket::SSL_AsyncSend(SSLSendIOServerData *data, notstd::HandleType h)
	{
		if (h)
			data->mUserCallback = h;
		data->SetHandle(IOSOCKET_MEMBER_BIND(&SSLSocket::OnSendAfterHandShake, this));

		std::string realDataToSend;
		auto buffer = data->GetDataBuffer();
		if (!WriteToSSLStream(buffer->mBegin, buffer->mLen, realDataToSend)
			|| realDataToSend.empty())
		{
			data->mUserCallback(nullptr, false, data, 0);
			return;
		}
		data->GetDataBuffer()->SetBufferSize(realDataToSend.c_str(), realDataToSend.size());
		AsyncSend(data);
	}

	void SSLSocket::SSL_AsyncRecv(SSLReceiveIOServerData *data, notstd::HandleType h)
	{
		if (h)
			data->mUserCallback = h;
		data->SetHandle(IOSOCKET_MEMBER_BIND(&SSLSocket::OnRecvAfterHandShake, this));
		AsyncRecv(data);
	}

	void SSLSocket::SSL_AsyncAccept(notstd::AcceptIOServerData *data, IOSocket *listenSocket, notstd::HandleType h)
	{
		if (h)
			mUserOnAcceptOrUserOnConnect = h;
		data->SetHandle(IOSOCKET_MEMBER_BIND(&SSLSocket::OnSSLAccept, this));
		AsyncAccept(data, listenSocket);
	}

	void SSLSocket::SSL_AsyncConnect(const NetAddress &addr, notstd::ConnectIOServerData *data, notstd::HandleType h)
	{
		if (h)
			mUserOnAcceptOrUserOnConnect = h;
		data->SetHandle(IOSOCKET_MEMBER_BIND(&SSLSocket::OnSSLConnect, this));
		AsyncConnect(addr, data);
	}
}
