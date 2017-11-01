#include "stdafx.h"
#include "netLib.h"
#include "BufferObject.h"
#include "notstd/sockbase.h"
#include "event2/bufferevent.h"
#include "event2/http.h"
#include "evhttp.h"

namespace runtime {

	////////////////////////////////////////////////////////////////////////////

	SimpleHttpConnectionObj::SimpleHttpConnectionObj()
	{
		notstd::NetEnviroment::Init();
	}

	SimpleHttpConnectionObj::~SimpleHttpConnectionObj()
	{
		notstd::NetEnviroment::Term();
	}

	runtimeObjectBase* SimpleHttpConnectionObj::doCall(doCallContext *context)
	{
		// 第1个参数：url
		// 第2个参数：超时时间（单位ms）
		// 第3个参数：hostname（如果需要）

		do {
		//	auto paramCount = context->GetParamCount();
		//	const char *url = nullptr;
		//	const char *hostname = nullptr;
		//	const char *urlpath = nullptr;
		//	uint32_t port = 80;
		//	uint32_t timeout = 0;

		//	if (paramCount < 1)
		//		break;

		//	url = context->GetStringParam(0);
		//	if (!url)
		//		break;
		//	auto urlresult = evhttp_uri_parse(url);
		//	if (!urlresult)
		//		break;
		//	hostname = evhttp_uri_get_host(urlresult);
		//	if (!hostname)
		//		break;
		//	port = static_cast<uint32_t>(evhttp_uri_get_port(urlresult));
		//	if (port == uint32_t(-1))
		//		port = 80;
		//	urlpath = evhttp_uri_get_path(urlresult);
		//	if (!urlpath || urlpath[0] == 0)
		//		urlpath = "/";

		//	if (paramCount >= 2)
		//		timeout = context->GetUint32Param(1);

		//	if (paramCount >= 3)
		//		hostname = context->GetStringParam(2);

		//	event_base *base;
		//	evhttp_connection *conn;
		//	evhttp_request *req;

		//	base = event_base_new();

		//	struct ResultInfo
		//	{
		//		bool succeed;
		//		event_base *base;
		//		int rCode;
		//		std::string data;
		//	} ri = { false, base, -1, };

		//	conn = evhttp_connection_base_new(base, nullptr, hostname, static_cast<unsigned short>(port));
		//	evhttp_connection_set_base(conn, base);
		//	req = evhttp_request_new([](evhttp_request *req, void *arg)
		//	{
		//		auto ri = reinterpret_cast<ResultInfo*>(arg);
		//		if (req)
		//		{
		//			auto rc = evhttp_request_get_response_code(req);
		//			ri->rCode = rc;
		//			if (rc >= 200 && rc < 400)
		//			{
		//				auto ss = evbuffer_get_length(req->input_buffer);
		//				ri->succeed = true;
		//				if (ss)
		//				{
		//					ri->data.resize(ss);
		//					memcpy(&ri->data[0], evbuffer_pullup(req->input_buffer, ss), ss);
		//				}
		//			}
		//		}
		//		event_base_loopbreak(ri->base);
		//	}, &ri);
		//	if (hostname)
		//		evhttp_add_header(req->output_headers, "host", hostname);
		//	evhttp_make_request(conn, req, EVHTTP_REQ_GET, url);
		//	if (timeout)
		//		evhttp_connection_set_timeout(req->evcon, timeout);
		//	event_base_dispatch(base);

		//	evhttp_connection_free(conn);
		//	event_base_free(base);

		//	if (ri.succeed)
		//	{
		//		auto rr = new ObjectModule<stringObject>;
		//		*rr->mVal = ri.data;
		//		return rr;
		//	}
		} while (0);

		return NullTypeObject::CreateNullTypeObject();
	};

	////////////////////////////////////////////////////////////////////////////

	TCPSocketReceiveObj::TCPSocketReceiveObj()
		: mTCPSocketObj(nullptr)
	{
	}

	TCPSocketReceiveObj::~TCPSocketReceiveObj()
	{
		if (mTCPSocketObj)
			mTCPSocketObj->Release();
	}

	runtimeObjectBase* TCPSocketReceiveObj::doCall(doCallContext *context)
	{
		// 接收的逻辑：
		// 第一个参数是Buffer
		// 第二个参数是接收字节数，带符号32位整数，接收到的数据追加到Buffer的尾部
		// 第三个参数是超时时间，单位毫秒，该参数可以省略，则直到
		// recv系统调用返回，本函数才返回；否则，接收到指定字节数，
		// 或者超时才返回，返回接收到的字节数（整形）。如果失败，
		// 返回<0的值

		BufferObject *buffObj;
		int count;
		int timeout = 0;
		auto paramCount = context->GetParamCount();

		char dataBuf[64 * 1024];

		do {
			if (paramCount < 2)
				break;
			auto *obj = context->GetParam(0);
			if (obj->GetObjectTypeId() != DT_object)
				break;
			obj = static_cast<objTypeObject*>(obj)->getInner();
			if (!obj || obj->GetObjectTypeId() != DT_buffer)
				break;
			buffObj = static_cast<BufferObject*>(obj);

			count = context->GetInt32Param(1);
			if (count < 0 || count > sizeof(dataBuf))
				break;
			// 如果一个字节都不接收，则返回0
			if (!count)
				return intObject::CreateIntObject(0);

			if (paramCount > 2)
				timeout = context->GetInt32Param(2);
			if (timeout < 0)
				break;

			DWORD to = timeout > 0 ? timeout : 0;
			int ret = mTCPSocketObj->mSock.ReceiveEx(dataBuf, count, &to);
			if (ret >= 0)
				buffObj->mBuffer.append(dataBuf, ret);
			return intObject::CreateIntObject(ret);
		} while (0);
		return intObject::CreateIntObject(-1);
	}

	////////////////////////////////////////////////////////////////////////////

	TCPSocketSendObj::TCPSocketSendObj()
		: mTCPSocketObj(nullptr)
	{
	}

	TCPSocketSendObj::~TCPSocketSendObj()
	{
		if (mTCPSocketObj)
			mTCPSocketObj->Release();
	}

	runtimeObjectBase* TCPSocketSendObj::doCall(doCallContext *context)
	{
		auto paramCount = context->GetParamCount();

		do {
			if (paramCount < 1)
				break;

			uint32_t off = 0;
			uint32_t size = -1;
			auto *obj = context->GetParam(0);
			if (obj->GetObjectTypeId() != DT_object)
				break;
			obj = static_cast<objTypeObject*>(obj)->getInner();
			if (!obj || obj->GetObjectTypeId() != DT_buffer)
				break;
			BufferObject *buffObj = static_cast<BufferObject*>(obj);
			uint32_t bufSize = static_cast<uint32_t>(buffObj->mBuffer.size());

			if (off >= bufSize)
				break;
			if (off + size < bufSize)
				size = bufSize - off;

			if (paramCount > 1)
				off = context->GetUint32Param(1);
			if (paramCount > 2)
				size = context->GetUint32Param(2);

			return intObject::CreateIntObject(
				mTCPSocketObj->mSock.SendEx(buffObj->mBuffer.c_str() + off, size));
		} while (0);
		return NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////

	TCPSocketDisconnectObj::TCPSocketDisconnectObj()
		: mTCPSocketObj(nullptr)
	{
	}

	TCPSocketDisconnectObj::~TCPSocketDisconnectObj()
	{
		if (mTCPSocketObj)
			mTCPSocketObj->Release();
	}

	runtimeObjectBase* TCPSocketDisconnectObj::doCall(doCallContext *context)
	{
		return intObject::CreateIntObject(mTCPSocketObj->mSock.Close());
	}

	////////////////////////////////////////////////////////////////////////////

	TCPSocketObj::TCPSocketObj()
	{
		notstd::NetEnviroment::Init();
	}

	TCPSocketObj::~TCPSocketObj()
	{
		notstd::NetEnviroment::Term();
	}

	runtimeObjectBase* TCPSocketObj::GetMember(const char *memName)
	{
		if (!strcmp("Disconnect", memName))
		{
			auto *r = new runtime::ObjectModule<TCPSocketDisconnectObj>;
			r->mTCPSocketObj = this;
			AddRef();
			return r;
		}
		else if (!strcmp("Send", memName))
		{
			auto *r = new runtime::ObjectModule<TCPSocketSendObj>;
			r->mTCPSocketObj = this;
			AddRef();
			return r;
		}
		else if (!strcmp("Receive", memName))
		{
			auto *r = new runtime::ObjectModule<TCPSocketReceiveObj>;
			r->mTCPSocketObj = this;
			AddRef();
			return r;
		}
		return baseTypeObject::GetMember(memName);
	}

	////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* tcpConnectObj::doCall(doCallContext *context)
	{
		const char *ip;
		uint32_t port;
		int timeout;
		auto paramCount = context->GetParamCount();

		// 需要三个参数，IP地址，端口号和超时时间，其中超时时间缺省为0，表示
		// 一直等待连接知道系统返回失败；否则该值表示超时时间对应的秒数

		do {
			if (paramCount < 2)
				break;
			ip = context->GetStringParam(0);
			if (!ip)
				break;
			port = context->GetUint32Param(1);
			if (port >= 65536)
				break;
			timeout = 0;
			if (paramCount == 3)
				timeout = context->GetInt32Param(2);
			if (timeout < 0)
				break;
			// 转成毫秒
			timeout *= 1000;
			auto *r = new ObjectModule<TCPSocketObj>;
			if (!r->mSock.Create())
				break;
			if (!r->mSock.Connect(notstd::NetAddress(ip, port), (long)timeout))
			{
				delete r;
				break;
			}
			return r;
		} while (0);
		return runtime::NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* inet_ntoaObj::doCall(doCallContext *context)
	{
		auto r = new ObjectModule<stringObject>;

		if (context->GetParamCount() != 1)
			return r;

		uint32_t v = context->GetUint32Param(0);
		const char *buf = inet_ntoa(*reinterpret_cast<in_addr*>(&v));
		if (!buf)
			return r;
		*r->mVal = buf;

		return r;
	}

	////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* AddressFromHostnameObj::doCall(doCallContext *context)
	{
		auto r = new ObjectModule<arrayObject>;
		r->AddSub(intObject::CreateIntObject(-1));

		if (context->GetParamCount() != 1)
			return r;

		const char *name = context->GetStringParam(0);
		if (!name)
			return r;
		notstd::IPV4Address addr;
		addr.FromDNS(name);
		static_cast<intObject*>(r->GetSub(0))->mVal = 0;
		r->AddSub(uintObject::CreateUintObject(*reinterpret_cast<uint32_t*>(addr.mAddr)));
		
		return r;
	}

	////////////////////////////////////////////////////////////////////////////

	NetworkByteOrderToHostByteOrderObj* NetworkByteOrderToHostByteOrderObj::Create(ByteOrderType t)
	{
		auto r = new ObjectModule<NetworkByteOrderToHostByteOrderObj>;
		r->mType = t;
		return r;
	}

	runtimeObjectBase* NetworkByteOrderToHostByteOrderObj::doCall(doCallContext *context)
	{
		do {
			if (context->GetParamCount() != 1)
				break;

			if (mType == BO_UINT16)
			{
				uint32_t orig = context->GetUint32Param(0);
				return uintObject::CreateUintObject(ntohs(static_cast<uint16_t>(orig)));
			}
			else if (mType == BO_UINT32)
			{
				uint32_t orig = context->GetUint32Param(0);
				return uintObject::CreateUintObject(ntohl(orig));
			}

			// 其它的目前不支持
		} while (0);
		return NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////
}
