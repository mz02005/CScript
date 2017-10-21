#pragma once
#include "ioServer-base.h"
#include "nslist.h"
#include "sockbase.h"
#include "notstd/stringHelper.h"
#include <functional>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include "event.h"

#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_MACOS)

///////////////////////////////////////////////////////////////////////////

#define IOSOCKET_MEMBER_BIND(addr,obj) std::bind(addr, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

///////////////////////////////////////////////////////////////////////////

namespace notstd {

	class IOSocket;
	class AcceptSocket;
	class IOServerProcessor;

	class NOTSTD_API AcceptIOServerData : public SocketIOServerData
	{
		friend class IOServer;
		friend class AcceptSocket;
		friend class IOServerProcessor;

	private:
		AcceptSocket *mAcceptResult;
		notstd::NetAddress mClient, mServer;

	public:
		AcceptIOServerData();
		
		void SetAcceptSocket(AcceptSocket *acceptSocket) {
			mAcceptResult = acceptSocket;
		}
		AcceptSocket* GetAcceptSocket() { return mAcceptResult; }

		const notstd::NetAddress& GetClientAddr() const { return mClient; }
		const notstd::NetAddress& GetServerAddr() const { return mServer; }
	};

	class IOHandleBase
	{
	public:
		virtual int GetFDHandle() = 0;
	};

	struct IORequest
	{
		int mFdHandle;
		IOServerData *mIOServerData;
	};

	class NOTSTD_API IOSocket : public IOHandleBase
	{
		friend class IOServerProcessor;

	protected:
		SocketHandle mSock;
		IOServer &mIOServer;

	public:
		IOSocket(IOServer &ioServer);

		SocketHandle& GetHandle() { return mSock; }
		virtual int GetFDHandle() override;

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

		virtual SOCKET Create(SocketType sockType = Stream,
			ProtocolType protocolType = Tcp, DWORD flags = 0) override;

		bool AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket,
			notstd::HandleType proc);
		bool AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket);
	};

	// 环形链表，用于给每个fd保存data链表
	// 这个链表缺省使用一个数组来进行操作，但是当该插入新的数据后，原来的数组不够大了
	// 则会采用动态分配内存的方式来获取数据
	template <typename T, int InitSize = 16>
	class CycleQueue
	{
	private:
		T mNodePool[InitSize];
		T *mNodes;
		int mHeader, mTail;
		uint32_t mPoolSize;

	public:
		CycleQueue()
			: mHeader(0)
			, mTail(0)
			, mNodes(mNodePool)
			, mPoolSize(InitSize)
		{
		}

		CycleQueue(CycleQueue &&cq)
			: mNodes(mNodePool)
		{
			mHeader = cq.mHeader;
			mTail = cq.mTail;
			if (cq.mNodes == cq.mNodePool)
			{
				int lastOne = mTail < mHeader ? InitSize : mTail;
				for (int i = mHeader; i < lastOne; i++)
					mNodes[i] = cq.mNodes[i];
				if (lastOne != InitSize)
				{
					for (int i = 0; i < mTail; i++)
						mNodes[i] = cq.mNodes[i];
				}
				mPoolSize = cq.mPoolSize;
			}
			else
			{
				mNodes = cq.mNodes;
			}
			cq.mNodes = cq.mNodePool;
			cq.mHeader = cq.mTail = 0;
		}

		~CycleQueue()
		{
			if (mNodes != mNodePool)
			{
				std::string ss;
				notstd::StringHelper::Format(ss, "mNodes=%llu, mNodePool=%llu\n",
					(uint64_t)mNodes, (uint64_t)mNodePool);
				std::cout << ss;
				free(mNodes);
			}
		}

		size_t size() const
		{
			return (mHeader > mTail ? mTail + (InitSize - mHeader) : mTail - mHeader);
		}

		void ExtendSpace()
		{
			std::cout << "ExtendSpace\n";
			// 为了计算方便（之后的位置调整），每次都是扩大一倍容量
			mPoolSize *= 2;
			if (mNodes == mNodePool)
			{
				mNodes = (T*)malloc(sizeof(T) * mPoolSize);
				memcpy(mNodes, mNodePool, sizeof(mNodePool));
			}
			else
			{
				mNodes = (T*)realloc(mNodes, sizeof(T) * mPoolSize);
			}

			if (mTail < mHeader)
			{
				memcpy(mNodes + mPoolSize / 2, mNodes, sizeof(T) * mTail);
				mTail = mPoolSize / 2 + mTail;
			}
		}

		bool empty() const
		{
			return (mHeader == mTail);
		}

		bool isFull() const
		{
			return ((mTail + 1) % mPoolSize) == mHeader;
		}

		T front()
		{
			assert(!empty());
			return mNodes[mHeader];
		}

		void pop_front()
		{
			assert(!empty());
			mHeader++;
			mHeader %= mPoolSize;
		}

		void push_back(const T &p)
		{
			if (isFull())
			{
				ExtendSpace();
			}
			mNodes[mTail++] = p;
			mTail %= mPoolSize;
		}

		void clear()
		{
			mHeader = mTail = 0;
		}

		class Iterator
		{
		public:
			uint32_t mIter;
			CycleQueue *mNode;

		public:
			Iterator(uint32_t p, CycleQueue *n)
				: mIter(p)
				, mNode(n)
			{
				if (n->empty())
					mIter = -1;
			}

			Iterator operator++(int)
			{
				uint32_t x = mIter++;
				x %= mNode->mPoolSize;
				if (mIter == mNode->mHeader)
					mIter = -1;
				return Iterator(x, mNode);
			}

			T operator*()
			{
				return mNode->mNodes[mIter];
			}

			friend bool operator == (const Iterator &t1, const Iterator &t2)
			{
				return t1.mIter == t2.mIter;
			}

			friend bool operator != (const Iterator &t1, const Iterator &t2)
			{
				return t1.mIter != t2.mIter;
			}
		};

		Iterator begin()
		{
			return Iterator(mHeader, this);
		}

		Iterator end()
		{
			return Iterator(mTail, this);
		}
	};

	// 一个线程对应一个处理器
	class NOTSTD_API IOServerProcessor
	{
	private:
		int mEpollHandle;
		IOServer &mIOServer;

		struct HandleIOCounter
		{
			enum
			{
				IO_EPOLLIN,
				IO_EPOLLOUT,

				IO_MAX_COUNT,
			};

			int mFdHandle;
			// 保存data
			CycleQueue<IOServerData*> mDataQueue[IO_MAX_COUNT];

			uint32_t mEpollEvent;
			HandleIOCounter()
				: mEpollEvent(0)
				, mFdHandle(-1)
			{
			}

			bool shouldDel() const {
				return mDataQueue[IO_EPOLLIN].empty() && mDataQueue[IO_EPOLLOUT].empty();
			}
		} mHandleIOCounter;
		std::unordered_map<int, HandleIOCounter> mIORequestMap;

	private:
		void OnConnect(int fd, notstd::IOServerData *data);
		void OnSend(int fd, notstd::IOServerData *data);
		void OnRecv(int fd, notstd::IOServerData *data);
		void OnAccept(int fd, notstd::IOServerData *data);

	public:
		IOServerProcessor(IOServer &ioServer);
		bool RunOneStep(IORequest &ioRequest);

		bool CreateProcessor();
		// 释放所有的数据，处理队列中没有处理掉的请求项
		void DestroyProcessor();
	};

	class IOServer
	{
		friend class IOServerProcessor;
		friend class IOSocket;
		friend class AcceptSocket;

	private:
		//std::condition_variable mNotifyCond;
		notstd::Event mNotifyEvent;
		std::mutex mNotifyMutex;

		std::mutex mRequestListMutex;
		// 需要一个列表来存放所有请求
		notstd::List<IORequest> mIORequestList;

	private:
		bool RunOneStep(IOServerProcessor &processor);
		void AddRequest(IORequest &&request);

	public:
		IOServer();
		virtual ~IOServer();
		
		void CreateIOServer();
		void DestroyIOServer();
		void BindToHandle(HANDLE h);
		void PostUserEvent(IOServerData *data);
		virtual void Run();
		virtual void PostQuit(IOServerQuitMessageData *quitData);
	};
}

#endif
