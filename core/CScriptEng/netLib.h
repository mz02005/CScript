#pragma once
#include "rtTypes.h"
#include "rtlib.h"
#include "notstd/sockbase.h"
#include <string.h>

namespace runtime {

	////////////////////////////////////////////////////////////////////////////

	class SimpleHttpConnectionObj : public runtime::baseTypeObject
	{
	public:
		SimpleHttpConnectionObj();
		virtual ~SimpleHttpConnectionObj();
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	////////////////////////////////////////////////////////////////////////////

	class TCPSocketObj : public runtime::baseTypeObject
	{
		friend class tcpConnectObj;
		friend class TCPSocketDisconnectObj;
		friend class TCPSocketSendObj;
		friend class TCPSocketReceiveObj;

	private:
		notstd::SocketHandle mSock;

	public:
		TCPSocketObj();
		virtual ~TCPSocketObj();
		virtual runtimeObjectBase* GetMember(const char *memName) override;
	};

	////////////////////////////////////////////////////////////////////////////

	class TCPSocketDisconnectObj : public runtime::baseTypeObject
	{
		friend class TCPSocketObj;

	private:
		TCPSocketObj *mTCPSocketObj;

	public:
		TCPSocketDisconnectObj();
		virtual ~TCPSocketDisconnectObj();
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	////////////////////////////////////////////////////////////////////////////

	class TCPSocketSendObj : public runtime::baseTypeObject
	{
		friend class TCPSocketObj;

	private:
		TCPSocketObj *mTCPSocketObj;

	public:
		TCPSocketSendObj();
		virtual ~TCPSocketSendObj();
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	////////////////////////////////////////////////////////////////////////////

	class TCPSocketReceiveObj : public runtime::baseTypeObject
	{
		friend class TCPSocketObj;

	private:
		TCPSocketObj *mTCPSocketObj;

	public:
		TCPSocketReceiveObj();
		virtual ~TCPSocketReceiveObj();
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	////////////////////////////////////////////////////////////////////////////
}
