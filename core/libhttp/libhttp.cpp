#include "stdafx.h"
#include "libhttp.h"
#include "httpServerNew.h"

extern "C" {
	typedef void* HttpServerHandle;
	HttpServerHandle STDCALL_API SimpleStartHttpServer(uint32_t port)
	{
		libhttpd::HttpServer *server = new libhttpd::HttpServer;
		libhttpd::HttpServConf conf;
		conf.port = static_cast<decltype(conf.port)>(port);
		if (server->StartHttpServer(&conf) >= 0)
			return reinterpret_cast<HttpServerHandle>(server);
		delete server;
		return nullptr;
	}

	void STDCALL_API SimpleStopHttpServer(HttpServerHandle handle)
	{
		if (handle)
		{
			auto server = reinterpret_cast<libhttpd::HttpServer*>(handle);
			server->StopHttpServer();
			delete server;
		}
	}
}

namespace libhttpd {
	HttpServConf::HttpServConf()
	{
		memset(this, 0, sizeof(*this));
		sizeOfMe = sizeof(*this);
		strcpy(ipv4Addr, "0.0.0.0");
	}

	HttpServer::HttpServer()
		: mHttpServerInner(new payHttpServer)
	{
	}

	HttpServer::~HttpServer()
	{
		StopHttpServer();
		delete mHttpServerInner;
	}

	int HttpServer::StartHttpServer(HttpServConf *conf)
	{
		return mHttpServerInner->StartPayHttpServer(conf) ? 0 : -1;
	}

	void HttpServer::StopHttpServer()
	{
		mHttpServerInner->StopPayHttpServer();
	}
}
