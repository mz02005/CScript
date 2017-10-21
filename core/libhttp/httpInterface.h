#pragma once
#include <inttypes.h>

#include "notstd/config.h"
#if defined(PLATFORM_MACOS)
#include <CoreServices/CoreServices.h>
#endif

namespace base {
	inline int32_t AtomicInc(int32_t *v)
	{
#if defined(PLATFORM_WINDOWS)
		return InterlockedIncrement(reinterpret_cast<LONG*>(v));
#elif defined(PLATFORM_MACOS)
		return IncrementAtomic(v) + 1;
#else
		return __sync_add_and_fetch(v, 1);
#endif
	}

	inline int32_t AtomicDec(int32_t *v)
	{
#if defined(PLATFORM_WINDOWS)
		return InterlockedDecrement(reinterpret_cast<LONG*>(v));
#elif defined(PLATFORM_MACOS)
		return DecrementAtomic(v) - 1;
#else
		return __sync_sub_and_fetch(v, 1);
#endif
	}
}

struct HttpConst
{
	enum
	{
		E_Generic = 1,
		E_BufOutOfRange,
		E_NotImpl,
		E_NotEnoughBufferSize,
		E_ParamError,
		E_AllreadyExists,
	};

	enum {
		UrlSize = 512,
		Version = 16,
	};
	
	// 声明了部分HTTP命令以及WebDAV扩展
	enum Verb
	{
		V_GET,
		V_POST,

		V_PUT,
		V_OPTIONS,
		V_HEAD,
		V_DELETE,
		V_TRACK,
		V_COPY,
		V_PROPFIND,
		V_PROPPATCH,
		V_MKCOL,
		V_MOVE,
		V_SEARCH,
	};
};

struct HttpRequestBaseInfo
{
	HttpConst::Verb verb;
	char url[HttpConst::UrlSize];
	char version[HttpConst::Version];
	const char *resLocalPosition;
	size_t posStringLen;
};

class RefObjBase;
class IHttpManager;
class IHttpResponse;
class IHttpRequest;

class RefObjBase
{
public:
	virtual long IncRef() = 0;
	virtual long DecRef() = 0;
};

enum StepFireCond
{
	SFC_READ,
	SFC_WRITE,
	SFC_OTHER,
	SFC_ASYNC,
};

enum HttpStage : int
{
	CS_WAIT_METHOD,
	CS_WAIT_HEADER,

	CS_WAIT_BODY,

	CS_DECODE,

	CS_POSITION_RESOURCE,

	CS_AUTHORITY,

	// 产生应答数据，该状态和之后的状态可能循环出现
	// 直到所有的数据都产生并且发送完毕
	CS_GENERATE_RESPONSE_DATA,

	CS_SEND_Header,
	CS_WAIT_HeaderSendOver,
	CS_ENCODE,
	CS_SEND_Body,
	CS_WAIT_BodySendOver,
	CS_WAIT_FOR_NEWROUND,

	CS_MAX,
};

enum StepResult
{
	NTD_CONTINUE = 0,
	NTD_COMPLETE = 1,
	NTD_ERROROCCURED = 2,
	NTD_WAIT = 3,
};

class IBufferPiece : public RefObjBase
{
public:
	virtual int AppendData(const void *buf, size_t s) = 0;
	virtual int GetBuffer(void *&buf, size_t &s) = 0;
	virtual uint64_t GetSize() = 0;
};

class IDataBuffer : public RefObjBase
{
public:
	virtual IBufferPiece* CreatePeice() = 0;
	virtual int AppendPiece(IBufferPiece *piece) = 0;
	virtual int InsertPieceBeforeHeader(IBufferPiece *piece) = 0;
	virtual IBufferPiece* RemoveHeaderPiece() = 0;
	virtual IBufferPiece* RemoveTailPiece() = 0;
	virtual void RemoveAll() = 0;
	virtual int GetFirstPiece() = 0;
	virtual int GetNextPiece(IBufferPiece *&piece) = 0;
	virtual uint64_t GetTotalSize() = 0;
};

class IHeader : public RefObjBase
{
public:
	virtual int AppendHeader(const char *header, const char *val) = 0;
	virtual int SetHeader(const char *header, const char *val) = 0;

	virtual int GetHeader(const char *name,
		char *buffer, size_t *bufSize) = 0;
	virtual int GetFirstHeader() = 0;
	virtual int GetNextHeader(char *name, size_t *nameSize, char *val, size_t *valSize) = 0;
	virtual int ClearAllHeader() = 0;
};

typedef void(*ConnectionDataDestoy)(void *data);
class IConnection : public RefObjBase
{
public:
	virtual void* CreateData(void *ptr, size_t s, ConnectionDataDestoy onDestroy = nullptr) = 0;
	virtual void* FindData(void *ptr) = 0;
	virtual int GoNextStage(HttpStage &stage) = 0;
	virtual StepResult SendSimpleTextRespond(int statusCode, const void *data, size_t s) = 0;
	virtual StepResult SendSimpleTextRespond(int statusCode, const char *data) = 0;
	virtual StepResult SendRespond(int statusCode, const char *contentType, const char *data, size_t s = -1,
		int(*OnBeforeSendResponse)(IConnection *conn, IHttpRequest *request, IHttpResponse *resp) = nullptr) = 0;
	virtual StepResult Send401Respond(const char *realm) = 0;
};

class IHttpResponse : public IHeader
{
public:
	virtual int SetStatusCode(int code) = 0;
	virtual int GetStatusCode() = 0;
	virtual IDataBuffer* GetResponseBuffer() = 0;
	virtual IHttpRequest* GetHttpRequest() = 0;
};

class IHttpRequest : public IHeader
{
public:
	virtual IDataBuffer* GetReceiveBuffer() = 0;

	virtual int GetHttpRequestBaseInfo(HttpRequestBaseInfo *info) = 0;
	virtual int SetResourceLocalPosition(const char *res, size_t len) = 0;

	// Param
	virtual int GetParam(const char *name,
		char *buffer, size_t *bufSize) = 0;
	virtual int GetFirstParam() = 0;
	virtual int GetNextParam(char *name, size_t *nameSize, char *val, size_t *valSize) = 0;

	virtual int DoBasicAuthenticate(const char *username, const char *password) = 0;
};

#define GETMODULECAPABILITY "GetModuleCapability"
#define DOSTEP "DoStep"
#define INITIALIZEMODULE "InitializeModule"
#define UNINITIALIZEMODULE "UninitializeModule"

extern "C" {
	typedef uint64_t(*GetModuleCapabilityProc)();
	typedef StepResult(*DoStepProc)(void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);
	typedef int (*InitializeModuleProc)();
	typedef int (*UninitializeModuleProc)();
}

template <typename T>
class ModuleHelper : public T
{
	long mRef;

public:
	ModuleHelper()
		: mRef(1)
	{
	}

	virtual ~ModuleHelper()
	{
	}

	virtual long IncRef()
	{
		return base::AtomicInc(reinterpret_cast<int32_t*>(&mRef));
		//return ::InterlockedIncrement(&mRef);
	}

	virtual long DecRef()
	{
		//auto r = ::InterlockedDecrement(&mRef);
		auto r = base::AtomicDec(reinterpret_cast<int32_t*>(&mRef));
		if (!r)
		{
			delete static_cast<T*>(this);
			return 0;
		}
		return r;
	}
};

template <typename T>
inline T* CreateRefObject()
{
	auto r = static_cast<T*>(new ModuleHelper<T>);
	return r;
}
