#include "stdafx.h"
#include "sockmgr.h"
#include <algorithm>
#include <process.h>
#include <Mswsock.h>
#include "stringHelper.h"
#include <assert.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

///////////////////////////////////////////////////////////////////////////////

IOCompleteData::IOCompleteData()
{
	memset(this, 0, sizeof(*this));
}

///////////////////////////////////////////////////////////////////////////////

DefaultCompleteData::DefaultCompleteData()
{
	mBuffer = &mIOBuffer;
}

///////////////////////////////////////////////////////////////////////////////

AdvIOBuffer::AdvIOBuffer(UINT defaultSize)
	: mBuf(NULL)
{
	Malloc(defaultSize);
	mBuf[0] = 0;
	mSize = 0;
}

AdvIOBuffer::~AdvIOBuffer()
{
	free(mBuf);
	mRealCap = mReportCap = mSize = 0;
}

bool AdvIOBuffer::Malloc(std::size_t len)
{
	// 指定的容量必须大于0
	if (!len)
		return false;

	mRealCap = (len + 15) / 16 * 16;
	mReportCap = len;

	mBuf = reinterpret_cast<char*>(realloc(mBuf, mRealCap));
	if (!mBuf)
		return false;
	
	return true;
}

UINT AdvIOBuffer::GetCap() const
{
	return mReportCap;
}

UINT AdvIOBuffer::GetSize() const
{
	return mSize;
}

char* AdvIOBuffer::GetBuffer()
{
	return mBuf;
}

void AdvIOBuffer::AppendData(const char *str, std::size_t len)
{
	std::size_t needSize = mSize + len;
	if (needSize > mRealCap)
		Malloc(needSize);
	if (needSize > mReportCap)
		mReportCap = needSize;
	memcpy(mBuf + mSize, str, len);
	mSize = needSize;
}

void AdvIOBuffer::SetLength(std::size_t s)
{
	if (mRealCap < s)
		Malloc(s);
	if (s > mReportCap)
		mReportCap = s;
	mSize = s;
}

void AdvIOBuffer::SetCap(std::size_t s)
{
	if (s > mRealCap)
		Malloc(s);
	mReportCap = s;
	if (mSize > mReportCap)
		mSize = mReportCap;
}

///////////////////////////////////////////////////////////////////////////////

IOOperationBase::~IOOperationBase()
{
}

///////////////////////////////////////////////////////////////////////////////

UINT WINAPI IOCompletionManager::CompletionThread(LPVOID lpParam)
{
	IOCompletionManager *pIOManager = reinterpret_cast<IOCompletionManager*>(lpParam);
	pIOManager->CompletionProc();
	return 0;
}

void IOCompletionManager::CompletionProc()
{
	ULONG_PTR completionKey;
	DWORD transferred;
	OVERLAPPED *ol;

	bool terminate = false;

	while (!terminate)
	{
		if (!::GetQueuedCompletionStatus(mIOCP, &transferred, &completionKey,
			&ol, INFINITE))
		{
			DWORD r = ::GetLastError();
			IOCompleteData *pData = reinterpret_cast<IOCompleteData*>(ol);
			if (pData)
			{
				if (pData->mOperation == IO_CONNECT)
				{
					pData->mIOOperation->OnConnect(false);
				}
				else
				{
					pData->mIOOperation->OnClose();
				}
				//switch (r)
				//{
				//case ERROR_NETNAME_DELETED:
				//case ERROR_OPERATION_ABORTED:
				//	pData->mSocket->OnClose();
				//	break;

				//default:
				//	pData->mSocket->OnConnect(false);
				//}
			}
			else
			{
				// 完成端口HANDLE已经被关闭了
				if (r == ERROR_ABANDONED_WAIT_0)
					break;
			}
		}
		else
		{
			IOCompleteData *pData = reinterpret_cast<IOCompleteData*>(ol);
			if (pData->mIOOperation)
			{
				if (pData->mIOOperation->IsError())
				{
					pData->mIOOperation->OnClose();
					continue;
				}
				if (!transferred) {
					if (IO_ACCEPT != pData->mOperation && pData->mOperation != IO_CONNECT)
					{
						pData->mIOOperation->OnClose();
						continue;
					}
				}
			}

			switch (pData->mOperation)
			{
			case IO_READ:
				pData->mOperation = 0;
				pData->mBuffer->SetLength(transferred);
				pData->mIOOperation->OnReceive(pData);
				break;

			case IO_WRITE:
				pData->hasDone += transferred;
				if (pData->hasDone > pData->tryToSend)
				{
					int m = 0;
					m++;
					break;
				}
				if (pData->hasDone == pData->tryToSend)
				{
					pData->mOperation = 0;
					pData->mIOOperation->OnSend(pData);
					pData->mBuffer->SetLength(0);
					pData->hasDone = 0;
				}
				else
				{
					pData->mIOOperation->SendPartial(pData);
				}
				break;

			case IO_CUSTUM:
				pData->mOperation = 0;
				pData->mIOOperation->OnCustumMsg(pData);
				break;

			case IO_CONNECT:
				pData->mOperation = 0;
				pData->mIOOperation->OnConnect(true);
				break;

			case IO_ACCEPT:
			{
				INT serverLen, clientLen;
				SockByAccept *sockByAccept = static_cast<SockByAccept*>(reinterpret_cast<Socket*>(pData->mIOOperation));

				if (::setsockopt(sockByAccept->GetHandle().GetHandle(), SOL_SOCKET,
					SO_UPDATE_ACCEPT_CONTEXT,
					reinterpret_cast<const char*>(&sockByAccept->mServerSock),
					sizeof(SOCKET)) != 0)
					break;

				(*NetEnviroment::GetAcceptExSockaddrs)(sockByAccept->mSockAddr,
					0, sizeof(NetAddress) + 16, sizeof(NetAddress) + 16,
					reinterpret_cast<LPSOCKADDR*>(&sockByAccept->mServer), &serverLen,
					reinterpret_cast<LPSOCKADDR*>(&sockByAccept->mClient), &clientLen);
				::getsockname(reinterpret_cast<Socket*>(pData->mIOOperation)->mSocket.GetHandle(),
					reinterpret_cast<sockaddr*>(&sockByAccept->mServer), &serverLen);
				::getpeername(reinterpret_cast<Socket*>(pData->mIOOperation)->mSocket.GetHandle(),
					reinterpret_cast<sockaddr*>(&sockByAccept->mClient), &clientLen);
				pData->mOperation = 0;
				pData->mIOOperation->OnAccept(sockByAccept->mClient, sockByAccept->mServer);
			}
				break;

			case IO_TERMINATE:
				pData->mOperation = 0;
				terminate = true;
				break;
			}
		}
	}
}

IOCompletionManager::IOCompletionManager()
	: mIOCP(NULL)
	, mIOCompletionThreads(NULL)
	, mThreadCount(0)
{
}

IOCompletionManager::~IOCompletionManager()
{
	StopManager();
}

bool IOCompletionManager::StartManager(DWORD minThread)
{
	StopManager();

	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	mThreadCount = si.dwNumberOfProcessors;

	if (minThread)
	{
		mThreadCount *= 2;
		if (mThreadCount < minThread)
			mThreadCount = minThread;
	}

	mIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, mThreadCount);
	if (!mIOCP)
		return false;

	mIOCompletionThreads = new HANDLE[mThreadCount];
	UINT i;
	for (i = 0; i < mThreadCount; i++)
	{
		mIOCompletionThreads[i] = (HANDLE)::_beginthreadex(NULL, 0,
			&IOCompletionManager::CompletionThread,
			reinterpret_cast<void*>(this), 0, NULL);
		if (!mIOCompletionThreads)
			break;
	}

	if (i != mThreadCount) {
		StopManager();
		return false;
	}

	return true;
}

void IOCompletionManager::StopManager()
{
	PostTerminateSignal();
	mThreadCount = 0;

	if (mIOCP) {
		::CloseHandle(mIOCP);
		mIOCP = NULL;
	}
}

void IOCompletionManager::Join()
{
	::WaitForMultipleObjects(mThreadCount, mIOCompletionThreads, TRUE, INFINITE);
	StopManager();
}

bool IOCompletionManager::BindIOHandle(HANDLE handle)
{
	return !!::CreateIoCompletionPort(
		handle, mIOCP, NULL, 0);
}

void IOCompletionManager::PostTerminateSignal()
{
	bool createThreadFail = false;
	if (!mThreadCount)
		return;
	IOCompleteData *data = new IOCompleteData[mThreadCount];
	for (UINT i = 0; i < mThreadCount; i++)
	{
		if (!mIOCompletionThreads[i]) {
			// 只要有一个为空，就表示是由于创建线程失败调用到这里的
			createThreadFail = true;
			break;
		}
		data[i].mOperation = IO_TERMINATE;
		::PostQueuedCompletionStatus(mIOCP, 0, static_cast<ULONG_PTR>(0),
			reinterpret_cast<LPOVERLAPPED>(data + i));
	}
	if (!createThreadFail && mThreadCount > 0)
		::WaitForMultipleObjects(mThreadCount, mIOCompletionThreads,
		TRUE, INFINITE);
	delete[] data;
	for (UINT i = 0; i < mThreadCount; i++)
	{
		if (mIOCompletionThreads[i])
			::CloseHandle(mIOCompletionThreads[i]);
	}
	if (mIOCompletionThreads) {
		delete[] mIOCompletionThreads;
		mIOCompletionThreads = NULL;
	}
}
