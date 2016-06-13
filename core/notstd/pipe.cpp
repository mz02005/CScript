#include "stdafx.h"
#include "pipe.h"
#include <process.h>

///////////////////////////////////////////////////////////////////////////////

PipeData::PipeData()
{
	memset(this, 0, sizeof(*this));
}

///////////////////////////////////////////////////////////////////////////////

PipeManager::PipeManager()
	: mThreadCount(0)
	, mConcurrentThreads(NULL)
{
}

PipeManager::~PipeManager()
{
	StopManager();
}

UINT WINAPI PipeManager::CompThread(LPVOID lParam)
{
	PipeManager *pipeManager = reinterpret_cast<PipeManager*>(lParam);
	pipeManager->CompletionProc();
	return 0;
}

void PipeManager::CompletionProc()
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
			DWORD le = ::GetLastError();
			switch (le)
			{
			case ERROR_BROKEN_PIPE:
				break;
			default:
				return;
			}
		}

		PipeData *pipeData = reinterpret_cast<PipeData*>(ol);
		switch (pipeData->operation)
		{
		case PIPE_OPERATION_READ:
			if (transferred)
			{
				pipeData->pipe->OnRead(pipeData->buffer, static_cast<std::size_t>(transferred));
			}
			else
			{
				// TODO: 看看是不是对方关闭了，需要测试一下
				pipeData->pipe->OnClose();
			}
			break;

		case PIPE_OPERATION_WRITE:
			if (transferred)
			{
				pipeData->pipe->OnWrite(static_cast<std::size_t>(transferred));
			}
			else
			{
				pipeData->pipe->OnClose();
			}
			break;

		case PIPE_OPERATION_ACCEPT:
			pipeData->pipe->OnAccept();
			break;

		case PIPE_OPERATION_TERMINATE:
			return;
		}
	}
}

bool PipeManager::StartManager(DWORD threadCount)
{
	StopManager();

	if (!threadCount)
	{
		SYSTEM_INFO si;
		::GetSystemInfo(&si);
		mThreadCount = si.dwNumberOfProcessors;
		mThreadCount *= 2;
	}
	else
	{
		mThreadCount = threadCount;
	}

	// 这里强制只允许一个线程来处理
	mIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, mThreadCount);
	if (!mIOCP)
		return false;

	mConcurrentThreads = new HANDLE[mThreadCount];
	DWORD i;
	for (i = 0; i < mThreadCount; i++)
	{
		mConcurrentThreads[i] = reinterpret_cast<HANDLE>(
			::_beginthreadex(NULL, 0, &PipeManager::CompThread,
			reinterpret_cast<void*>(this), 0, NULL));
		if (!mConcurrentThreads[i])
			break;
	}

	if (i != mThreadCount)
	{
		mIOCP.Close();
		for (DWORD j = 0; j <= i; j++)
		{
			::CloseHandle(mConcurrentThreads[j]);
		}
		delete [] mConcurrentThreads;
		mConcurrentThreads = NULL;
		return false;
	}
	
	return true;
}

bool PipeManager::IsManagerStopped()
{
	DWORD r = ::WaitForMultipleObjects(mThreadCount, mConcurrentThreads, TRUE, 0);
	return (r != WAIT_TIMEOUT);
}

void PipeManager::StopManager()
{
	PipeData pipeData;
	memset(&pipeData, 0, sizeof(pipeData));
	pipeData.operation = PIPE_OPERATION_TERMINATE;

	if (mConcurrentThreads && mThreadCount)
	{
		for (DWORD i = 0; i < mThreadCount; i++)
		{
			::PostQueuedCompletionStatus(mIOCP, 0, static_cast<ULONG_PTR>(0), &pipeData);
		}

		::WaitForMultipleObjects(mThreadCount, mConcurrentThreads, TRUE, INFINITE);
		for (DWORD i = 0; i < mThreadCount; i++)
		{
			::CloseHandle(mConcurrentThreads[i]);
		}
		delete[] mConcurrentThreads;
		mConcurrentThreads = NULL;
		mThreadCount = 0;
	}

	mIOCP.Close();
}

void PipeManager::Join()
{
	::WaitForMultipleObjects(mThreadCount, mConcurrentThreads, TRUE, INFINITE);
}

///////////////////////////////////////////////////////////////////////////////

void NamedPipe::OnAccept()
{
}

void NamedPipe::OnRead(const char *buffer, std::size_t size)
{
}

void NamedPipe::OnWrite(std::size_t size)
{
}

void NamedPipe::OnClose()
{
}

NamedPipe::NamedPipe()
{
	mPipeDataRead.pipe = this;
	mPipeDataWrite.pipe = this;
}

NamedPipe::~NamedPipe()
{
	mNamePipe.Close();
}

bool NamedPipe::CreateNamedPipeServer(PipeManager *pipeManager, const std::wstring &pipeName)
{
	std::wstring acturePipeName = L"\\\\.\\pipe\\";
	acturePipeName += pipeName;
	mNamePipe = ::CreateNamedPipeW(acturePipeName.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 
		PIPE_TYPE_BYTE, 1, 
		PIPE_MAX_WRITE_BUFFER, PIPE_MAX_READ_BUFFER, 0, NULL);
	if (!mNamePipe)
		return false;
	if (!::CreateIoCompletionPort(mNamePipe, pipeManager->mIOCP, static_cast<ULONG_PTR>(0), 0))
	{
		mNamePipe.Close();
		return false;
	}
	mPipeManager = pipeManager;
	mPipeDataRead.operation = PIPE_OPERATION_ACCEPT;
	if (!::ConnectNamedPipe(mNamePipe, reinterpret_cast<LPOVERLAPPED>(&mPipeDataRead)))
	{
		DWORD lastError = GetLastError();
		if (lastError == ERROR_PIPE_LISTENING || lastError == ERROR_IO_PENDING)
		{
			return true;
		}
		if (lastError !=	ERROR_PIPE_CONNECTED)
		{
			OnAccept();
			return true;
		}
		mNamePipe.Close();
	}
	return true;
}

bool NamedPipe::CreatenamedPipeClient(PipeManager *pipeManager, const std::wstring &pipeName)
{
	std::wstring acturePipeName = L"\\\\.\\pipe\\";
	acturePipeName += pipeName;
	mNamePipe = ::CreateFileW(acturePipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, 
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (!mNamePipe)
		return false;
	if (!::CreateIoCompletionPort(mNamePipe, pipeManager->mIOCP, static_cast<ULONG_PTR>(0), 0))
	{
		mNamePipe.Close();
		return false;
	}
	mPipeManager = pipeManager;
	return true;
}

bool NamedPipe::Write(const char *buffer, std::size_t size)
{
	if (size > PIPE_MAX_WRITE_BUFFER)
		return false;
	mPipeDataWrite.operation = PIPE_OPERATION_WRITE;
	memcpy(mPipeDataWrite.buffer, buffer, size);
	if (!::WriteFile(mNamePipe, mPipeDataWrite.buffer, static_cast<DWORD>(size), NULL,
		reinterpret_cast<LPOVERLAPPED>(&mPipeDataWrite)))
	{
		DWORD lastError = ::GetLastError();
		if (lastError != ERROR_IO_PENDING)
			return false;
	}
	return true;
}

bool NamedPipe::SyncWrite(const char *buffer, std::size_t size)
{
	const char *p = buffer;
	std::size_t left = size;
	while (left)
	{
		DWORD writed = 0;
		if (!::WriteFile(mNamePipe, p, static_cast<DWORD>(left), &writed, NULL)
			|| !writed)
			return false;
		p += writed;
		left -= writed;
	}
	return true;
}

bool NamedPipe::Read()
{
	mPipeDataRead.operation = PIPE_OPERATION_READ;
	if (!::ReadFile(mNamePipe, mPipeDataRead.buffer, PIPE_MAX_READ_BUFFER, NULL, 
		reinterpret_cast<LPOVERLAPPED>(&mPipeDataRead)))
	{
		DWORD lastError = ::GetLastError();
		if (lastError != ERROR_IO_PENDING)
			return false;
	}
	return true;
}

void NamedPipe::Close()
{
	mNamePipe.Close();
}

///////////////////////////////////////////////////////////////////////////////
