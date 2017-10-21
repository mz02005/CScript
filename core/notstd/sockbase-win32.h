#pragma once
#include "config.h"

extern "C"
{
	typedef BOOL(WINAPI *AcceptExProc)(
		SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
	typedef void (WINAPI *GetAcceptExSockaddrsProc)(
		PVOID, DWORD, DWORD, DWORD, LPSOCKADDR*, LPINT, LPSOCKADDR*, LPINT);
	typedef BOOL(PASCAL *ConnectExProc)(SOCKET, const struct sockaddr*, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);
}

namespace notstd {

	class NOTSTD_API NetEnviroment {
	public:
		static AcceptExProc AcceptEx;
		static GetAcceptExSockaddrsProc GetAcceptExSockaddrs;
		static ConnectExProc ConnectEx;

		static bool GetExtFuncPointer();

	public:
		static bool Init();
		static void Term();
	};

}
