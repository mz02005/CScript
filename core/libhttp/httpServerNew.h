/*
Add by mz02005
*/

#pragma once
#include <thread>
#include <mutex>
#include <map>
#include <list>
#include <unordered_map>
#include <assert.h>
#include "event2/event.h"
#include "event2/bufferevent.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "libhttp.h"
#include "notstd/sockbase.h"
#ifdef PLATFORM_WINDOWS
#include <WinSock2.h>
#endif

class payHttpServer;
class Connection;
class SSLConnection;
class SSLContext;

#define LIBHTTP_LOGOUT

typedef std::unordered_map<std::string, std::string> FormData;

class FormDataHelper
{
public:
	static void WriteToString(const FormData &fd, std::string &s);
};

#define CONNECT_IDLE_TIME	(1000 * 120)

class Connection : public IConnection
{
	friend class HttpResponse;
	friend class payHttpServer;

protected:
	// 底层调用发送数据的实现
	virtual int ConnSendData(const char *d, size_t l);
	virtual void ConnOnRecv(const char *d, size_t l);
	virtual int CreateSocketBuffer(event_base *eb);

protected:
	bool mDel;
	bool mKeepAlive;
	
	// http服务器
	payHttpServer *mServer;

	SOCKET mSock;
	bufferevent *mBufferEvent;
	
	HttpStage mHttpStage;

	struct UserdataEntry
	{
		void *ptr;
		void *userdata;
		ConnectionDataDestoy onDestroy;
	};
	std::unordered_map<void*, UserdataEntry> mUserdataTable;
	void DestroyData();

private:
	static void OnSend(bufferevent *bev, void *arg);
	static void OnRecv(bufferevent *bev, void *arg);
	static void OnEvent(bufferevent *bev, short event, void *arg);

	void OnRecv(const void *buf, size_t s);
	void OnSend();
	void OnMaintenance(DWORD tick);

private:
	// 框架在某个阶段如果找不到任何可以处理过程，则会调用本过程
	StepResult DefaultStep(StepFireCond &cond, HttpStage &stage, 
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);

	static StepResult DoPositionResource(void *userData, IConnection *param, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);
	StepResult DoPositionResource(StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);

	static StepResult DoHeaderReceive(void *userData, IConnection *param, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);
	StepResult DoHeaderReceive(StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);

	static StepResult DoReceiveBodyInner(void *userData, IConnection *param, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);
	StepResult DoReceiveBodyInner(StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);
			
	// 分析http方法标记
	bool ParseHttpMethod(const char *s, size_t l);

	// 分析一行Header
	bool ParseSingleHeader(const char *s, size_t l);

	inline void IncStage(HttpStage &stage);

	DWORD mLastSend, mLastRecv;

	// 缓冲接收到的数据
	std::string mDataBuff;
	// 为了快速找到http协议中的关键数据位置，缓存上一次搜索的位置
	size_t mLastFind;
	uint64_t mDataLength;
	
	HttpRequest *mHttpRequest;
	HttpResponse *mHttpResponse;

private:
	void ResetState();

	//void DoResponse();
	//void SendResponse(int statusCode, const std::string &s, const std::string &contentType = "");
	//void SendHtmlResponse(int statusCode, const std::string &htmlData);
	//void SendHtmlResponseWithPrompt(int statusCode, const std::string &title, const std::string &content);
	//bool ParseFormData(FormData &formData);

	StepResult SendResponse(int statusCode, const std::string &s, const std::string &contentType = "");
	
public:
	Connection();
	virtual ~Connection();
	
	virtual void* CreateData(void *ptr, size_t s, ConnectionDataDestoy onDestroy = nullptr);
	virtual void* FindData(void *ptr);
	virtual int GoNextStage(HttpStage &stage);
	virtual StepResult SendSimpleTextRespond(int statusCode, const void *data, size_t s);
	virtual StepResult SendSimpleTextRespond(int statusCode, const char *data);
	virtual StepResult SendRespond(int statusCode, const char *contentType, const char *data, size_t s = -1,
		int(*OnBeforeSendResponse)(IConnection *conn, IHttpRequest *request, IHttpResponse *resp) = nullptr);
	virtual StepResult Send401Respond(const char *realm);

	void SetDelete() {
		if (mBufferEvent)
		{
			bufferevent_free(mBufferEvent);
			mBufferEvent = nullptr;
		}
		mDel = true;
	}

	void SetBaseInfo(payHttpServer *server, SOCKET sock);

	static Connection* NewConnection(payHttpServer *server, SOCKET sock);
	static bool InitializeConn(Connection *conn);
	static bool DestroyConnection(Connection *conn);
};

class payHttpServer
{
	friend class Connection;
	friend class SSLConnection;

	struct ModuleEntry
	{
		// 有些操作节点不是从模块中加载的，那样在释放的时候就不需要释放节点
		int doNotFreeModule;
		void* module;
		std::string modulePathName;
		GetModuleCapabilityProc GetModuleCapability;
		DoStepProc DoStep;
		InitializeModuleProc InitializeModule;
		UninitializeModuleProc UninitializeModule;
	};

private:
	bool mStartupOK;

	SSLContext *mSSLContext;

	libhttpd::HttpServConf mConf;
	SOCKET mListenSocket;

	std::list<ModuleEntry> mModuleEntryList;
	std::vector<std::list<ModuleEntry*>> mWorkflow;

	event_base *mEventBase;
	event *mListenEvent;
	
	//std::mutex mConnListMutex;
	typedef std::list<Connection*> HttpConnectionList;
	HttpConnectionList mHttpConnList;
	event *mMaintenanceTimer;
	timeval mMaintenanceVal;
	int mTimerCounter;

	std::thread mWorkingThread;

private:
	static void OnAccept(evutil_socket_t sockfd, short event_type, void *arg);
	static void OnMaintenanceTimer(int fd, short event, void *arg);
	void OnMaintenanceTimer(int fd, short event);

private:
	void LoadExtModule();
	void AddInnerExtModule(GetModuleCapabilityProc p1, DoStepProc p2);
	void UnloadExtModule();
	void AddConnection(Connection *conn);
	void LockConnectionList();
	void UnlockConnectionList();
	void OnCleanup();

	int DoStep(Connection *conn, StepFireCond &cond, HttpStage &stage, IHttpRequest *request, IHttpResponse *response, const void *buf, size_t s);

public:
	static int OnGetStringBuffer(const std::string &str, char *&buffer, size_t *&bufSize);

	payHttpServer();
	~payHttpServer();

	void LogOut(const char *format, ...);
	void LogOut(const char *format, va_list valist);

	bool StartPayHttpServer(libhttpd::HttpServConf *conf);
	void StopPayHttpServer();
};
