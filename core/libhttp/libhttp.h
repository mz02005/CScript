#pragma once
#include <inttypes.h>
#include "httpInterface.h"

#if defined(WIN32)
#ifdef LIBHTTP_EXPORTS
#define LIBHTTP_API __declspec(dllexport)
#else
#define LIBHTTP_API __declspec(dllimport)
#endif
#else
#define LIBHTTP_API
#endif

class payHttpServer;

namespace libhttpd {

	enum
	{
		MaxPath = 260,
		MAX_PRIVATE_KEY_PASSWORD_LENGTH = 64,
	};

	enum
	{
		HSC_HTTPS = 1 << 0,
		HSC_HTTPS_VERIFY_CLIENT = 1 << 1,
	};

	struct LIBHTTP_API HttpServConf
	{
		uint16_t sizeOfMe;
		uint16_t port;
		uint32_t flags;
		char ipv4Addr[16];
		void *userData;
		char certificateFilePathName[MaxPath];
		char privateKeyFilePathName[MaxPath];
		char privateKeyPassword[MAX_PRIVATE_KEY_PASSWORD_LENGTH];
		char clientCertFilePathName[MaxPath];
		struct HttpProcessorEntry
		{
			GetModuleCapabilityProc GetModuleCapability;
			DoStepProc DoStep;
		} entry;

		HttpServConf();
	};

	class LIBHTTP_API HttpServer
	{
	private:
		payHttpServer *mHttpServerInner;

	public:
		HttpServer(void);
		virtual ~HttpServer();

		int StartHttpServer(HttpServConf *conf);
		void StopHttpServer();
	};
}
