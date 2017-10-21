#include "stdafx.h"
#include "core/libhttp/libhttp.h"
#include <thread>
#include <chrono>
#include <iostream>
#include "notstd/simpleTool.h"
#include "notstd/filesystem.h"

int main(int argc, char* argv[])
{
	libhttpd::HttpServer theMainHttpServer;
	libhttpd::HttpServConf backendConf;
	backendConf.port = 10088;
	backendConf.userData = nullptr;
	backendConf.entry.GetModuleCapability = nullptr;
	backendConf.entry.DoStep = nullptr;

	notstd::WinProfile profile;
	std::string profPath = notstd::AppHelper::GetAppPath();
	profPath += ".conf";
	if (profile.OpenProfile(profPath.c_str()) < 0)
	{
		printf("Open profile fail\n");
#ifdef PLATFORM_WINDOWS
		pi_ncpy_s(backendConf.certificateFilePathName, MAX_PATH, "C:\\Users\\mz020\\Desktop\\test.crt", _TRUNCATE);
		pi_ncpy_s(backendConf.privateKeyFilePathName, MAX_PATH, "C:\\Users\\mz020\\Desktop\\test.key", _TRUNCATE);
		pi_ncpy_s(backendConf.clientCertFilePathName, MAX_PATH, "C:\\users\\mz020\\Desktop\\clienttest.crt", _TRUNCATE);
#else
		pi_ncpy_s(backendConf.certificateFilePathName, MAX_PATH, "/home/mz/test.crt", _TRUNCATE);
		pi_ncpy_s(backendConf.privateKeyFilePathName, MAX_PATH, "/home/mz/test.key", _TRUNCATE);
		pi_ncpy_s(backendConf.clientCertFilePathName, MAX_PATH, "/home/mz/clienttest.crt", _TRUNCATE);
#endif
		strcpy(backendConf.privateKeyPassword, "000000");
	}
	else
	{
		pi_ncpy_s(backendConf.certificateFilePathName, libhttpd::MaxPath,
			profile._GetProfileString("General", "CertificateFile", "/home/mz/test.crt").c_str(), _TRUNCATE);
		pi_ncpy_s(backendConf.privateKeyFilePathName, libhttpd::MaxPath,
			profile._GetProfileString("General", "PrivateKeyFile", "/home/mz/test.key").c_str(), _TRUNCATE);
		pi_ncpy_s(backendConf.privateKeyPassword, libhttpd::MAX_PRIVATE_KEY_PASSWORD_LENGTH,
			profile._GetProfileString("General", "PrivateKeyPassword", "000000").c_str(), _TRUNCATE);
		pi_ncpy_s(backendConf.clientCertFilePathName, libhttpd::MaxPath,
			profile._GetProfileString("General", "ClientCertFile", "/home/mz/clienttest.key").c_str(), _TRUNCATE);

		backendConf.port = static_cast<uint16_t>(profile._GetProfileUint32("General", "Port", 10088));
		if (profile._GetProfileInt32("General", "ssl", 0))
			backendConf.flags |= libhttpd::HSC_HTTPS;
		if (profile._GetProfileInt32("General", "VerifyClient", 0))
			backendConf.flags |= libhttpd::HSC_HTTPS_VERIFY_CLIENT;
	}
	
	if (theMainHttpServer.StartHttpServer(&backendConf) < 0)
	{
		std::cerr << "Start httpserver fail\n";
		return -1;
	}

	for (;;)
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	return 0;
}
