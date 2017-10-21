#pragma once
#include "simpleTool.h"
#include "sockbase.h"
#include "objbase.h"

struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;

struct ssl_st;
typedef struct ssl_st SSL;

struct bio_set;
typedef struct bio_st BIO;

namespace notstd {
	///////////////////////////////////////////////////////////////////////////

	class IOServer;
	class IOServerData;
	
	///////////////////////////////////////////////////////////////////////////

#if defined(PLATFORM_WINDOWS)
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
#else
	class NOTSTD_API IOErrorCode
	{
		bool mVal;

	public:
		IOErrorCode();
		IOErrorCode(bool succeeded);
		operator bool() const;
	};
#endif

	////////////////////////////////////////////////////////////////////////////

	enum
	{
		MAX_TCP_BUFFER_SIZE = 65500,
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

	///////////////////////////////////////////////////////////////////////////

	class IOServerData;
	typedef std::function<void(notstd::IOServer*, const notstd::IOErrorCode&, IOServerData *data, size_t)> HandleType;
#if defined(PLATFORM_WINDOWS)
	template class NOTSTD_API std::function<void(notstd::IOServer*, const notstd::IOErrorCode&, IOServerData *data, size_t)>;
#endif

	///////////////////////////////////////////////////////////////////////////

	class NOTSTD_API IOServerData
#if defined(PLATFORM_WINDOWS)
		: public OVERLAPPED
#endif
	{
		friend class IOServer;

	public:
		enum {
			// 需要立即处理的IO消息
			IOS_FLAG_IMM = 0x80000000,
		};
		enum
		{
			IOS_QUIT = IOS_FLAG_IMM,
			IOS_CONNECT = 1,
			IOS_RECEIVE,
			IOS_SEND,
			IOS_ACCEPT,
			IOS_TIMER = (IOS_ACCEPT + 1) | IOS_FLAG_IMM,
		};

	protected:
		uint32_t mType;
		HandleType mHandleFunc;

	public:
		IOServerData(uint32_t iosType);
		virtual ~IOServerData();

		uint32_t GetDataType() const { return mType; }

		void SetHandle(HandleType h);
		HandleType GetHandle() const { return mHandleFunc; }
		virtual void OnFunc(IOServer *ioServer, const IOErrorCode &e, 
			IOServerData *data, size_t trans);
	};

	///////////////////////////////////////////////////////////////////////////

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

	///////////////////////////////////////////////////////////////////////////
}
