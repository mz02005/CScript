#pragma once

// 完成端口模型实现的socket设施

#include "ioCompletionManager.h"
#include "sockbase.h"

class NOTSTD_API Socket : public IOOperationBase
{
	friend class IOCompletionManager;
	friend class IOManagerUseSystemThreadPool;
	friend class SockByAccept;
	
protected:
	IOCompletionManager *mIOManager;

protected:
	SocketHandle mSocket;

protected:
	virtual void OnCustumMsg(IOCompleteData *completeData);
	virtual void OnReceive(IOCompleteData *completeData);
	virtual void OnSend(IOCompleteData *completeData);
	virtual void OnConnect(bool connectOK);
	virtual void OnAccept(const NetAddress &client, const NetAddress &serv);
	virtual void OnClose();
	virtual bool IsError();

	virtual bool SendPartial(IOCompleteData *completeData);

public:
	Socket();
	virtual ~Socket();

	virtual bool Connect(IOCompleteData *completeData, const std::string &ipAddr, PORT_T port);
	virtual bool Connect(IOCompleteData *completeData, const NetAddress &netAddr);
	virtual bool Receive(IOCompleteData *completeData);
	virtual bool Send(IOCompleteData *completeData);
	bool SendSync(IOCompleteData *completeData);
	bool SendSync(const char *buf, std::size_t size);

	// 这两个函数没有经过测试，不保证正确性
	// RecvFrom的返回值addrFrom应该会在OnReceive返回时由系统填充
	// 保证该参数的生命周期和有效性是需要特别注意的
	virtual bool RecvFrom(IOCompleteData *completeData, NetAddress *addrFrom);
	virtual bool SendTo(IOCompleteData *completeData, const NetAddress *addrTo);

	virtual bool Create(IOCompletionManager *ioManager, SocketType nSocketType = Stream, 
		ProtocolType nProtocolType = Tcp, DWORD dwFlags = 0);
	virtual bool Close();

	// 可将内部状态置为某个值，然后发送通知让底层处理
	// 最后系统会调用OnCustumMsg函数
	virtual bool PostCustumMsg(IOCompleteData *completeData);

	SocketHandle& GetHandle();
};

class NOTSTD_API SockByAccept : public Socket {
	friend class IOCompletionManager;
	friend class IOManagerUseSystemThreadPool;

private:
	SOCKET mServerSock;

	// 储存Accept返回的服务端客户端地址
	char mSockAddr[(sizeof(NetAddress) + 16) * 2];

	NetAddress mServer, mClient;

public:
	virtual bool Accept(IOCompleteData *completeData, Socket &sockServer);
};
