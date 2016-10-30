#pragma once
#include "simpleTool.h"
#include "sockbase.h"
#include "objbase.h"

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

struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;

struct ssl_st;
typedef struct ssl_st SSL;

struct bio_set;
typedef struct bio_st BIO;

namespace notstd {
	class IOServer;

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

	enum
	{
		MAX_TCP_BUFFER_SIZE = 65500,
	};

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IOErrorCode
	{
	private:
		bool mSucceeded;
		DWORD mErrorCode;

	public:
		IOErrorCode(bool succeeded);

		operator bool() const;

		DWORD GetErrorCode() { return mErrorCode; }

		std::string what() const;
	};

	///////////////////////////////////////////////////////////////////////////

	class IOServerData;
	template class NOTSTD_API std::function<void(notstd::IOServer*, const notstd::IOErrorCode&, IOServerData *data, size_t)>;
	typedef std::function<void(notstd::IOServer*, const notstd::IOErrorCode&, IOServerData *data, size_t)> HandleType;

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

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IOServerData : public OVERLAPPED
	{
		friend class IOServer;

	public:
		enum
		{
			IOS_QUIT,
			IOS_CONNECT,
			IOS_RECEIVE,
			IOS_SEND,
			IOS_ACCEPT,
			IOS_TIMER,
		};

	protected:
		uint32_t mType;
		HandleType mHandleFunc;

	public:
		IOServerData(uint32_t iosType);
		virtual ~IOServerData();

		void SetHandle(HandleType h);
		HandleType GetHandle() const { return mHandleFunc; }
		virtual void OnFunc(IOServer *ioServer, const IOErrorCode &e, IOServerData *data, size_t trans);
	};

	struct NOTSTD_API IOServerDataBuffer
	{
	public:
		char mBuf[1400];
		char *mBegin;
		// mData一般为nullptr，不过如果设置的数据块比较大的话，就会分配内存到这个区域来发送了
		// mCap不能超过MAX_TCP_BUFFER_SIZE
		char *mData;
		size_t mCap, mLen;

	public:
		IOServerDataBuffer();
		~IOServerDataBuffer();

		void Clear();
		void CopyData(const IOServerDataBuffer *other);

		void SetBufferSize(const void *buf, size_t size);
		void SetBufferCap(size_t size);
		size_t GetCap() const { return mCap; }
		size_t GetSize() const { return mLen; }
	};

	class SocketIOServerData;
	class ConnectIOServerData;
	class SendIOServerData;
	class ReceiveIOServerData;
	class AcceptIOServerData;

	// 说明：这里采用成员的方式，并通过构造函数来决定事件处理和缓存的
	// 创建，是为了不使用模板
	class NOTSTD_API SocketIOServerData : public IOServerData
	{
		friend class IOSocket;
		friend class IOServer;

	protected:
		IOServerDataBuffer mDataBuffer;
		SOCKET mSock;

	public:
		SocketIOServerData(uint32_t type);
		virtual ~SocketIOServerData();

		virtual void OnFunc(IOServer *ioServer, 
			const IOErrorCode &e, IOServerData *data, size_t trans);

		IOServerDataBuffer* GetDataBuffer() { return &mDataBuffer; }

		SOCKET GetSocket() { return mSock; }
	};
	
	class NOTSTD_API IOServerQuitMessageData : public IOServerData
	{
	public:
		IOServerQuitMessageData();
	};

	class NOTSTD_API ConnectIOServerData : public SocketIOServerData
	{
	public:
		ConnectIOServerData();
	};

	class NOTSTD_API ReceiveIOServerData : public SocketIOServerData
	{
	public:
		ReceiveIOServerData();
	};

	class NOTSTD_API SendIOServerData : public SocketIOServerData
	{
	public:
		SendIOServerData();
	};

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

		SOCKET Create(SocketType sockType = Stream,
			ProtocolType protocolType = Tcp, DWORD flags = 0);

		int Close();

		bool AsyncConnect(const NetAddress &addr, ConnectIOServerData *data, notstd::HandleType proc);
		bool AsyncConnect(const NetAddress &addr, ConnectIOServerData *data);
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
		IOServerQuitMessageData mQuitData;
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
		virtual void PostQuit();
	};

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IOTimer
	{
		friend class TimerData;

	private:
		HANDLE mTimeQueueTimer;
		IOServer &mIOServer;
		UINT mDelay;
		//TimerData mTimeData;
		TimerData *mTimerData;
		// 是否仅激发一次
		bool mOnce;

		static void CALLBACK TimeCallBack(PVOID parameter, BOOLEAN timerOrWaitFired);
		bool CreateTimer();

	public:
		IOTimer(IOServer &ioServer);
		~IOTimer();

		void CloseTimer();

		bool CreateTimer(uint32_t delay, TimerData *timeData, notstd::HandleType proc, bool once = false);
		bool CreateTimer(uint32_t delay, bool once = false);
	};

	///////////////////////////////////////////////////////////////////////////

#define IOSOCKET_MEMBER_BIND(addr,obj) std::bind(addr, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

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
