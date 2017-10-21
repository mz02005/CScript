#include "stdafx.h"
#include "ioServer-posix.h"
#include <thread>

#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_MACOS)
#include <sys/epoll.h>

namespace notstd {

	////////////////////////////////////////////////////////////////////////////

	AcceptIOServerData::AcceptIOServerData()
		: SocketIOServerData(IOS_ACCEPT)
		, mAcceptResult(nullptr)
	{
	}

	////////////////////////////////////////////////////////////////////////////

	IOErrorCode::IOErrorCode()
		: mVal(false)
	{
	}

	IOErrorCode::IOErrorCode(bool succeeded)
		: mVal(!succeeded)
	{
	}

	IOErrorCode::operator bool() const
	{
		return mVal;
	}

	////////////////////////////////////////////////////////////////////////////

	IOSocket::IOSocket(IOServer &ioServer)
		: mIOServer(ioServer)
	{
	}

	int IOSocket::GetFDHandle()
	{
		return mSock.GetHandle();
	}

	SOCKET IOSocket::Create(SocketType sockType,
		ProtocolType protocolType, DWORD flags)
	{
		SOCKET r = mSock.Create(sockType, protocolType, flags);
		if (r >= 0)
		{
			if (!mSock.enableNoneBlockingMode(true))
			{
				mSock.Close();
				return -1;
			}
		}
		return r;
	}

	int IOSocket::Close()
	{
		return mSock.Close();
	}

	bool IOSocket::AsyncConnect(const NetAddress &addr, ConnectIOServerData *data, notstd::HandleType proc,
		const NetAddress &local)
	{
		if (proc)
			data->SetHandle(proc);

		if (connect(mSock.GetHandle(),
			reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0)
		{
			if (errno != EINPROGRESS)
				return false;
		}
		else
		{
			proc(&mIOServer, true, data, 0);
			return true;
		}

		IORequest iorequest;
		iorequest.mFdHandle = mSock.GetHandle();
		iorequest.mIOServerData = data;
		mIOServer.AddRequest(std::move(iorequest));

		return true;
	}

	bool IOSocket::AsyncConnect(const NetAddress &addr, ConnectIOServerData *data,
		const NetAddress &local)
	{
		return AsyncConnect(addr, data, nullptr, local);
	}

	int IOSocket::Connect()
	{
		return -1;
	}

	bool IOSocket::AsyncRecv(ReceiveIOServerData *data, notstd::HandleType proc)
	{
		if (proc)
			data->SetHandle(proc);

		IORequest iorequest;
		iorequest.mFdHandle = mSock.GetHandle();
		iorequest.mIOServerData = data;

		mIOServer.AddRequest(std::move(iorequest));

		return true;
	}

	bool IOSocket::AsyncRecv(ReceiveIOServerData *data)
	{
		return AsyncRecv(data, nullptr);
	}

	int IOSocket::Recv()
	{
		return false;
	}

	bool IOSocket::AsyncSend(SendIOServerData *data, notstd::HandleType proc)
	{
		if (proc)
			data->SetHandle(proc);
		
		IORequest iorequest;
		iorequest.mFdHandle = mSock.GetHandle();
		iorequest.mIOServerData = data;
		mIOServer.AddRequest(std::move(iorequest));

		return true;
	}

	bool IOSocket::AsyncSend(SendIOServerData *data)
	{
		return AsyncSend(data, nullptr);
	}

	int IOSocket::Send()
	{
		return -1;
	}

	////////////////////////////////////////////////////////////////////////////

	AcceptSocket::AcceptSocket(IOServer &ioServer)
		: IOSocket(ioServer)
	{
	}

	SOCKET AcceptSocket::Create(SocketType sockType, 
		ProtocolType protocolType, DWORD flags)
	{
		// 仅仅返回0，而不做任何事情，之后会在Accept成功后，Attach真的上去
		return 0;
	}

	bool AcceptSocket::AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket,
		notstd::HandleType proc)
	{
		data->SetAcceptSocket(this);
		if (proc)
			data->SetHandle(proc);

		IORequest iorequest;
		iorequest.mFdHandle = listenSocket->GetHandle().GetHandle();
		iorequest.mIOServerData = data;

		mIOServer.AddRequest(std::move(iorequest));

		return true;
	}

	bool AcceptSocket::AsyncAccept(AcceptIOServerData *data, IOSocket *listenSocket)
	{
		return AsyncAccept(data, listenSocket, nullptr);
	}
	
	////////////////////////////////////////////////////////////////////////////

	IOServerProcessor::IOServerProcessor(IOServer &ioServer)
		: mIOServer(ioServer)
		, mEpollHandle(-1)
	{
	}

	bool IOServerProcessor::CreateProcessor()
	{
		assert(mEpollHandle < 0);
		mEpollHandle = epoll_create(1);
		return (mEpollHandle >= 0);
	}

	void IOServerProcessor::DestroyProcessor()
	{
		for (auto iter = mIORequestMap.begin(); iter != mIORequestMap.end(); iter++)
		{
			auto &ioCounter = iter->second;
			for (int i = 0; i < HandleIOCounter::IO_MAX_COUNT; i++)
			{
				auto &dataQueue = ioCounter.mDataQueue[i];
				for (auto queIter = dataQueue.begin(); queIter != dataQueue.end(); queIter++)
				{
					(*queIter)->OnFunc(&mIOServer, false, *queIter, 0);
				}
			}
		}
		mIORequestMap.clear();
	}

	bool IOServerProcessor::RunOneStep(IORequest &ioRequest)
	{
		// 缺省会处理Close和错误，并且为水平触发
		static uint32_t basicEvent = EPOLLERR | EPOLLHUP;

		if (ioRequest.mFdHandle >= 0 && ioRequest.mIOServerData)
		{
			auto iter = mIORequestMap.find(ioRequest.mFdHandle);

			bool isNew = false;
			if (iter == mIORequestMap.end())
			{
				isNew = true;
				mIORequestMap.insert(std::make_pair(ioRequest.mFdHandle, HandleIOCounter()));
				iter = mIORequestMap.find(ioRequest.mFdHandle);

				iter->second.mFdHandle = ioRequest.mFdHandle;
				iter->second.mEpollEvent = basicEvent;
			}
			auto &ioCounter = iter->second;

			auto dataType = ioRequest.mIOServerData->GetDataType();
			switch (dataType)
			{
			case IOServerData::IOS_RECEIVE:
				ioCounter.mEpollEvent |= EPOLLIN;
				ioCounter.mDataQueue[HandleIOCounter::IO_EPOLLIN].push_back(ioRequest.mIOServerData);
				break;
			case IOServerData::IOS_SEND:
				ioCounter.mEpollEvent |= EPOLLOUT;
				ioCounter.mDataQueue[HandleIOCounter::IO_EPOLLOUT].push_back(ioRequest.mIOServerData);
				break;
			case IOServerData::IOS_CONNECT:
				// 连接成功后，socket变为可写状态
				ioCounter.mEpollEvent |= EPOLLOUT;
				ioCounter.mDataQueue[HandleIOCounter::IO_EPOLLOUT].push_back(ioRequest.mIOServerData);
				break;
			case IOServerData::IOS_ACCEPT:
				ioCounter.mEpollEvent |= EPOLLIN;
				ioCounter.mDataQueue[HandleIOCounter::IO_EPOLLIN].push_back(ioRequest.mIOServerData);
				break;

			default:
				std::cerr << "Invalid data type " << dataType << std::endl;
				return false;
			}

			epoll_event ee;
			ee.events = ioCounter.mEpollEvent;
			ee.data.ptr = &ioCounter;
			if (epoll_ctl(mEpollHandle, isNew ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, 
				ioCounter.mFdHandle, &ee) < 0)
			{
				std::cerr << "epoll_ctl fail, errno=" << errno << std::endl;
				return false;
			}
		}

		// 开始等待操作完成
		static const int eeCount = 16;
		epoll_event ee[eeCount];
		// 由于在之前等待新Request的时候已经用了等待时间，这里就不在指定超时时间了
		int r = epoll_wait(mEpollHandle, ee, eeCount, 0);
		if (r > 0)
		{
			for (int i = 0; i < r; i++)
			{
				HandleIOCounter *ioCounter = reinterpret_cast<HandleIOCounter*>(ee[i].data.ptr);
				assert(ioCounter);
				notstd::IOServerData *serData = nullptr;
				
				bool needModify = false;

				if (ee[i].events & EPOLLIN)
				{
					serData = ioCounter->mDataQueue[HandleIOCounter::IO_EPOLLIN].front();
					ioCounter->mDataQueue[HandleIOCounter::IO_EPOLLIN].pop_front();

					switch (serData->GetDataType())
					{
					case IOServerData::IOS_RECEIVE:
						OnRecv(ioCounter->mFdHandle, serData);
						break;

					case IOServerData::IOS_ACCEPT:
						OnAccept(ioCounter->mFdHandle, serData);
						break;

					default:
						return false;
					}
					if (ioCounter->mDataQueue[HandleIOCounter::IO_EPOLLIN].empty())
					{
						ee[i].events &= ~EPOLLIN;
						needModify = true;
					}
				}
				if (ee[i].events & EPOLLOUT)
				{
					serData = ioCounter->mDataQueue[HandleIOCounter::IO_EPOLLOUT].front();
					ioCounter->mDataQueue[HandleIOCounter::IO_EPOLLOUT].pop_front();

					switch (serData->GetDataType())
					{
					case IOServerData::IOS_CONNECT:
						OnConnect(ioCounter->mFdHandle, serData);
						break;

					case IOServerData::IOS_SEND:
						OnSend(ioCounter->mFdHandle, serData);
						break;

					default:
						return false;
					}
					if (ioCounter->mDataQueue[HandleIOCounter::IO_EPOLLOUT].empty())
					{
						ee[i].events &= ~EPOLLOUT;
						needModify = true;
					}
				}
				if (ee[i].events & EPOLLERR
					|| ee[i].events & EPOLLRDHUP)
				{
					epoll_ctl(mEpollHandle, EPOLL_CTL_DEL, ioCounter->mFdHandle, &ee[i]);

					// 如果出现了这个，则对该fd下的所有data，都执行失败回调
					auto iter = mIORequestMap.find(ioCounter->mFdHandle);
					if (iter != mIORequestMap.end())
					{
						for (int i = 0; i < HandleIOCounter::IO_MAX_COUNT; i++)
						{
							auto &dataQueue = iter->second.mDataQueue[i];
							for (auto iterQue = dataQueue.begin(); iterQue != dataQueue.end(); iterQue++)
							{
								(*iterQue)->OnFunc(&mIOServer, false, *iterQue, 0);
							}
						}
						mIORequestMap.erase(iter);
					}

					return true;
				}

				if (needModify)
				{
					bool del = ioCounter->shouldDel();
					epoll_ctl(mEpollHandle, del ? EPOLL_CTL_DEL : EPOLL_CTL_MOD, 
						ioCounter->mFdHandle, &ee[i]);
					if (del)
						mIORequestMap.erase(ioCounter->mFdHandle);
				}
			}
		}

		return true;
	}

	void IOServerProcessor::OnConnect(int fd, notstd::IOServerData *data)
	{
		int result;
		socklen_t len = sizeof(result);
		bool succeeded =
			getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &len) >= 0
			&& result == 0;
		data->OnFunc(&mIOServer, succeeded, data, 0);
	}

	void IOServerProcessor::OnAccept(int fd, notstd::IOServerData *data)
	{
		AcceptIOServerData *acceptData = static_cast<AcceptIOServerData*>(data);
		AcceptSocket *acceptSocket = acceptData->GetAcceptSocket();

		SOCKLEN_T len = sizeof(acceptData->mClient);
		int newSock = accept(fd, (sockaddr*)&acceptData->mClient, &len);
		if (newSock < 0)
		{
			data->OnFunc(&mIOServer, false, data, 0);
			return;
		}

		acceptSocket->mSock.Attach(newSock);
		acceptSocket->mSock.enableNoneBlockingMode(true);
		data->OnFunc(&mIOServer, true, data, 0);
	}

	void IOServerProcessor::OnRecv(int fd, notstd::IOServerData *data)
	{
		ReceiveIOServerData *sockData = static_cast<ReceiveIOServerData*>(data);
		IOServerDataBuffer *dataBuffer = sockData->GetDataBuffer();
		int result = recv(fd, dataBuffer->mBuf, dataBuffer->mCap, 0);
		if (result <= 0)
		{
			data->OnFunc(&mIOServer, false, data, 0);
			return;
		}

		dataBuffer->mLen = result;
		dataBuffer->mBegin = dataBuffer->mBuf;
		data->OnFunc(&mIOServer, true, data, result);
	}

	void IOServerProcessor::OnSend(int fd, notstd::IOServerData *data)
	{
		SocketIOServerData *sockData = static_cast<SocketIOServerData*>(data);
		IOServerDataBuffer *dataBuffer = sockData->GetDataBuffer();
		int result = send(fd, dataBuffer->mBegin, dataBuffer->mLen, 0);
		if (result <= 0)
		{
			data->OnFunc(&mIOServer, false, data, 0);
			return;
		}

		dataBuffer->mBegin += result;
		dataBuffer->mLen -= result;

		if (!dataBuffer->mLen)
		{
			dataBuffer->mBegin = dataBuffer->mData;
			data->OnFunc(&mIOServer, true, data, dataBuffer->mCap);
		}
		else
		{
			// 由于没有发送完整，所以还要再发送
			IORequest iorequest;
			iorequest.mFdHandle = fd;
			iorequest.mIOServerData = data;
			mIOServer.AddRequest(std::move(iorequest));
		}
	}

	////////////////////////////////////////////////////////////////////////////

	IOServer::IOServer()
	{
	}

	IOServer::~IOServer()
	{
		// TODO: 需要处理所有没有处理完的东西
	}

	void IOServer::BindToHandle(HANDLE h)
	{
		// linux下不需要显式绑定，由执行具体操作的异步函数来进行绑定操作
	}

	void IOServer::CreateIOServer()
	{
		mNotifyEvent.CreateEvent(false);
	}

	void IOServer::DestroyIOServer()
	{
		mNotifyEvent.CloseEvent();
	}

	void IOServer::Run()
	{
		IOServerProcessor proc(*this);
		if (!proc.CreateProcessor())
			return;
		for (; RunOneStep(proc); )
		{
		}
		proc.DestroyProcessor();
	}

	void IOServer::PostQuit(IOServerQuitMessageData *quitData)
	{
		{
			std::lock_guard<std::mutex> locker(mRequestListMutex);
			IORequest request;
			request.mFdHandle = -1;
			request.mIOServerData = quitData;
			mIORequestList.AddTail(request);
		}

		//mNotifyCond.notify_one();
		mNotifyEvent.SetEvent();
	}
	
	void IOServer::AddRequest(IORequest &&request)
	{
		{
			std::lock_guard<std::mutex> locker(mRequestListMutex);
			mIORequestList.AddTail(request);
		}
		//mNotifyCond.notify_one();

		mNotifyEvent.SetEvent();
	}

	bool IOServer::RunOneStep(IOServerProcessor &processor)
	{
		IORequest oneRequest = { -1, nullptr };

		//std::unique_lock<std::mutex> locker(mNotifyMutex);
		//if (mNotifyCond.wait_for(locker, std::chrono::milliseconds(100)) != std::cv_status::timeout)
		do {
			mNotifyEvent.WaitObject(100);

			{
				std::lock_guard<std::mutex> locker(mRequestListMutex);
				//assert(!mIORequestList.IsEmpty());
				if (mIORequestList.IsEmpty())
					break;
				oneRequest = mIORequestList.RemoveTail();
			}

			// 如果是需要立即处理的消息，则……
			auto dataType = oneRequest.mIOServerData->GetDataType();
			if (dataType & IOServerData::IOS_FLAG_IMM)
			{
				if (dataType == IOServerData::IOS_QUIT)
					return false;
				oneRequest.mIOServerData->OnFunc(this, false, oneRequest.mIOServerData, 0);
			}
		} while (0);
		
		return processor.RunOneStep(oneRequest);
	}

	////////////////////////////////////////////////////////////////////////////
}

#endif
