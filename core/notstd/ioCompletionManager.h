#pragma once

#include "config.h"
#include "sockbase.h"

namespace notstd {
	enum {
		IO_NOUSE,
		IO_TERMINATE,
		IO_READ,
		IO_WRITE,
		IO_READFROM,
		IO_WRITETO,
		IO_CONNECT,
		IO_ACCEPT,
		IO_CUSTUM,
	};

	///////////////////////////////////////////////////////////////////////////////

	class NOTSTD_API Buffer
	{
	public:
		virtual uint32_t GetCap() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual char* GetBuffer() = 0;
		virtual void AppendData(const char *str, std::size_t s) = 0;
		virtual void SetLength(std::size_t s) = 0;
		virtual void SetCap(std::size_t s) = 0;
	};

	class IOOperationBase;

	struct NOTSTD_API IOCompleteData : public OVERLAPPED
	{
		DWORD mOperation;
		IOOperationBase *mIOOperation;
		Buffer *mBuffer;
		WSABUF buf;
		UINT hasDone;
		UINT tryToSend;

		IOCompleteData();
	};

	class NOTSTD_API IOCompletionManager
	{
		friend class Socket;
		friend class Serial;

	private:
		HANDLE mIOCP;

		UINT mThreadCount;
		HANDLE *mIOCompletionThreads;

	private:
		static UINT WINAPI CompletionThread(LPVOID lpParam);
		void CompletionProc();
		void PostTerminateSignal();

	public:
		IOCompletionManager();
		virtual ~IOCompletionManager();

		virtual bool StartManager(DWORD minThread = 0);
		virtual void StopManager();
		virtual void Join();
		virtual bool BindIOHandle(HANDLE handle);

		HANDLE GetHandle() { return mIOCP; }
	};

	class NOTSTD_API IOOperationBase {
		friend class IOCompletionManager;

	protected:
		virtual void OnCustumMsg(IOCompleteData *completeData) = 0;
		virtual void OnReceive(IOCompleteData *completeData) = 0;
		virtual void OnSend(IOCompleteData *completeData) = 0;
		virtual void OnConnect(bool connectOK) = 0;
		virtual void OnAccept(const notstd::NetAddress &client, const notstd::NetAddress &serv) = 0;
		virtual void OnClose() = 0;
		virtual bool IsError() = 0;

		virtual bool SendPartial(IOCompleteData *completeData) = 0;

	public:
		virtual ~IOOperationBase();
	};

	class NOTSTD_API AdvIOBuffer : public Buffer {
	protected:
		char *mBuf;
		// 对外报告的容量
		uint32_t mReportCap;
		// 实际的容量
		uint32_t mRealCap;
		// 数据量
		uint32_t mSize;

	protected:
		bool Malloc(std::size_t len);

	public:
		virtual uint32_t GetCap() const;
		virtual uint32_t GetSize() const;
		virtual char* GetBuffer();
		virtual void AppendData(const char *str, std::size_t len);
		virtual void SetLength(std::size_t s);
		virtual void SetCap(std::size_t s);

		AdvIOBuffer(UINT defaultSize = 4096);
		virtual ~AdvIOBuffer();
	};

	struct NOTSTD_API DefaultCompleteData : public IOCompleteData
	{
		AdvIOBuffer mIOBuffer;

	public:
		DefaultCompleteData();
	};

}
