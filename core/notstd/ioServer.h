#pragma once
#include "simpleTool.h"
#include "sockbase.h"

namespace notstd {
	class IOServer;

	///////////////////////////////////////////////////////////////////////////

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

	template struct NOTSTD_API std::pair<char*, size_t>;
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
		virtual void OnFunc(IOServer *ioServer, const IOErrorCode &e, IOServerData *data, size_t trans);
	};

	class NOTSTD_API IOServerDataWithCache: public IOServerData
	{
	public:
		char mBuf[1400];
		char *mBegin;
		// mData一般为nullptr，不过如果设置的数据块比较大的话，就会分配内存到这个区域来发送了
		// mCap不能超过MAX_TCP_BUFFER_SIZE
		char *mData;
		size_t mCap, mLen;

	public:
		IOServerDataWithCache(uint32_t t);
		virtual ~IOServerDataWithCache();

		void SetSendBuffer(const void *d, size_t len);

		std::pair<char*, size_t> GetBufferAndSize();
		std::pair<char*, size_t> GetBufferAndCap();
	};

	class NOTSTD_API SocketIOServerData : public IOServerDataWithCache
	{
		friend class IOSocket;
	protected:
		SOCKET mSock;

	public:
		SocketIOServerData(uint32_t type);
		virtual void OnFunc(IOServer *ioServer, const IOErrorCode &e, IOServerData *data, size_t trans);
	};

	class NOTSTD_API IOServerQuitMessageData : public SocketIOServerData
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

		template <typename HandleProc>
		void AsyncConnect(const NetAddress &addr, ConnectIOServerData *data, HandleProc proc)
		{
			data->SetHandle(proc);
			AsyncConnect(addr, data);
		}
		void AsyncConnect(const NetAddress &addr, ConnectIOServerData *data);
		int Connect();

		template <typename HandleProc>
		void AsyncRecv(ReceiveIOServerData *data, HandleProc proc)
		{
			data->SetHandle(proc);
			AsyncRecv(data);
		}
		void AsyncRecv(ReceiveIOServerData *data);
		int Recv();

		template <typename HandleProc>
		void AsyncSend(SendIOServerData *data, HandleProc proc)
		{
			data->SetHandle(proc);
			AsyncSend(data);
		}
		void AsyncSend(SendIOServerData *data);
		int Send();
	};

	class NOTSTD_API AcceptSocket : public IOSocket
	{
	public:
		AcceptSocket(IOServer &ioServer);

		template <typename HandleProc>
		void AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket, HandleProc proc)
		{
			data->SetHandle(proc);
			AsyncAccept(data, listenSocket);
		}
		void AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket);
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
		MMRESULT mMMResult;
		IOServer &mIOServer;
		UINT mDelay;
		TimerData mTimeData;

		static void CALLBACK TimeCallBack(UINT timeId, UINT msg, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
		{
			IOTimer *ioTimer = reinterpret_cast<IOTimer*>(user);
			assert(timeId == static_cast<decltype(timeId)>(ioTimer->mMMResult));
			ioTimer->mIOServer.PostUserEvent(&ioTimer->mTimeData);
		}

		void CreateTimer();

	public:
		IOTimer(IOServer &ioServer);
		~IOTimer();

		void CloseTimer();

		template <typename HandleProc>
		void CreateTimer(uint32_t delay, HandleProc proc)
		{
			mTimeData.SetHandle(proc);
			CreateTimer(delay);
		}
		void CreateTimer(uint32_t delay);
	};

	///////////////////////////////////////////////////////////////////////////

#define IOSOCKET_MEMBER_BIND(addr,obj) std::bind(addr, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

	///////////////////////////////////////////////////////////////////////////
}
