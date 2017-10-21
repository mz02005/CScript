#pragma once
#include "ioServer-base.h"

namespace notstd {
	///////////////////////////////////////////////////////////////////////////

#define IOSOCKET_MEMBER_BIND(addr,obj) std::bind(addr, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API AcceptIOServerData : public SocketIOServerData
	{
		friend class IOServer;
		friend class AcceptSocket;

	private:
		SOCKET mServerSocket;
		notstd::NetAddress mClient, mServer;

		// 储存Accept返回的服务端客户端地址
		char mSockAddr[(sizeof(NetAddress) + 16) * 2];

	public:
		AcceptIOServerData();

		const notstd::NetAddress& GetClientAddr() const { return mClient; }
		const notstd::NetAddress& GetServerAddr() const { return mServer; }
	};

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API std::exception;
	class NOTSTD_API notstdException : public std::exception
	{
	protected:
		char *mBuf;
		size_t mLen;

	protected:
		char* adjustRoom(size_t s);

	public:
		notstdException();
		notstdException(const char *what);
		virtual ~notstdException();
		virtual const char *what() const;
	};

	class NOTSTD_API winSystemError : public notstdException
	{
	private:
		DWORD mError;

		void FormatErrorMessage();

	public:
		winSystemError();
		winSystemError(DWORD errCode);
	};

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API SSLContext
	{
	private:
		SSL_CTX *mSSLCtx;

	public:
		SSLContext();
		~SSLContext();

		SSL_CTX* GetSSLContext();
		bool UsePrivateKeyFile(const char *filePathName, char *password = nullptr);
		bool UseCertificateFile(const char *filePathName);

		bool SetNoVerify();
	};

	class IOTimer;
	class NOTSTD_API TimerData : public IOServerData
	{
		friend class IOTimer;

	private:
		IOTimer *mIOTimer;

	public:
		TimerData();
		virtual void OnFunc(IOServer *ioServer, const IOErrorCode &e,
			IOServerData *data, size_t trans);

		IOTimer* GetIOTimer() { return mIOTimer; }
	};

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IOSocket
	{
	protected:
		SocketHandle mSock;
		IOServer &mIOServer;

	public:
		IOSocket(IOServer &ioServer);

		SocketHandle& GetHandle() { return mSock; }

		virtual SOCKET Create(SocketType sockType = Stream,
			ProtocolType protocolType = Tcp, DWORD flags = 0);

		virtual int Close();

		bool AsyncConnect(const NetAddress &addr, ConnectIOServerData *data, notstd::HandleType proc,
			const NetAddress &local = notstd::NetAddress("0.0.0.0", 0));
		bool AsyncConnect(const NetAddress &addr, ConnectIOServerData *data,
			const NetAddress &local = notstd::NetAddress("0.0.0.0", 0));
		int Connect();

		bool AsyncRecv(ReceiveIOServerData *data, notstd::HandleType proc);
		bool AsyncRecv(ReceiveIOServerData *data);
		int Recv();

		bool AsyncSend(SendIOServerData *data, notstd::HandleType proc);
		bool AsyncSend(SendIOServerData *data);
		int Send();
	};

	class NOTSTD_API AcceptSocket : public IOSocket
	{
	public:
		AcceptSocket(IOServer &ioServer);

		bool AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket,
			notstd::HandleType proc);
		bool AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket);
	};

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IOServer
	{
	private:
		Handle<NormalHandleType> mIOServer;

	public:
		IOServer();
		virtual ~IOServer();

		void CreateIOServer();
		void DestroyIOServer();
		void BindToHandle(HANDLE h);
		void PostUserEvent(IOServerData *data);
		virtual bool RunOneStep();
		virtual void Run();
		virtual void PostQuit(IOServerQuitMessageData *quitData);
	};

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IOTimer
	{
		friend class TimerData;
		friend class IOServer;

	private:
		HANDLE mTimeQueueTimer;
		IOServer &mIOServer;
		uint32_t mDueTime;
		uint32_t mPeriod;
		TimerData *mTimerData;

		static void CALLBACK TimeCallBack(PVOID parameter, BOOLEAN timerOrWaitFired);

		virtual bool BeforeOnTimer();
		virtual void AfterOnTimer();

	public:
		IOTimer(IOServer &ioServer);
		~IOTimer();

		void CloseTimer(bool waitOver = true);

		// dueTime表示自调用本函数起，第一次激发的时间段
		// period表示从第二次被激发开始，间隔激发的时间段
		// 废弃原有的once参数
		bool CreateTimer(uint32_t dueTime, uint32_t period,
			TimerData *timeData, notstd::HandleType proc);
		bool CreateTimer(uint32_t dueTime, uint32_t period);
	};

	///////////////////////////////////////////////////////////////////////////
	// For ssl

	class SSLSocket;

	class NOTSTD_API SSLReceiveIOServerData : public notstd::ReceiveIOServerData
	{
		friend class SSLSocket;

	private:
		notstd::HandleType mUserCallback;
	};

	class NOTSTD_API SSLSendIOServerData : public notstd::SendIOServerData
	{
		friend class SSLSocket;

	private:
		notstd::HandleType mUserCallback;
	};

	class NOTSTD_API SSLSocket : public notstd::AcceptSocket
	{
	private:
		bool mHasHandShake;
		bool mMoreRead;

		// SSL相关
		SSL_CTX *mSSLContext;
		SSL *mSSL;
		enum { RECV, SEND };
		BIO *mBIO[2];

		notstd::SendIOServerData mSendDataForSSLHandShake;
		notstd::ReceiveIOServerData mRecvDataForSSLHandShake;
		char mRecvBuffer[1024];
		char mSendBuffer[1024];

		notstd::HandleType mUserOnHandShake;
		notstd::HandleType mUserOnAcceptOrUserOnConnect;

	private:
		void OnSSLReceive(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans);
		void OnSSLSend(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans);
		void OnSSLConnect(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans);
		void OnSSLAccept(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans);
		void OnSendAfterHandShake(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans);
		void OnRecvAfterHandShake(notstd::IOServer *ioServer, const notstd::IOErrorCode &e,
			notstd::IOServerData *data, size_t trans);
		bool ReadFromSSLStream(const char *o, size_t s, std::string &data);
		bool WriteToSSLStream(const char *toWrite, size_t s, std::string &data);

	public:
		SSLSocket(notstd::IOServer &ioServer, SSL_CTX *sslctx,
			notstd::HandleType onHandShake = nullptr);
		virtual ~SSLSocket();
		void SSL_AsyncSend(SSLSendIOServerData *data, notstd::HandleType h = nullptr);
		void SSL_AsyncRecv(SSLReceiveIOServerData *data, notstd::HandleType h = nullptr);
		void SSL_AsyncAccept(notstd::AcceptIOServerData *data,
			IOSocket *listenSocket, notstd::HandleType h = nullptr);
		void SSL_AsyncConnect(const NetAddress &addr, notstd::ConnectIOServerData *data, notstd::HandleType h = nullptr);
	};

	///////////////////////////////////////////////////////////////////////////
}
