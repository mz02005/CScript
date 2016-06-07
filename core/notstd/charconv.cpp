#include "stdafx.h"
#include "charconv.h"

#ifdef PLATFORM_WINDOWS
#define UNICODE_CODEC "UCS-2LE"
#else
#define UNICODE_CODEC "UCS-2BE"
#endif

namespace notstd {

std::wstring ICONVext::utf8ToUnicode(const std::string &str)
{
	std::wstring r;

	if (str.empty())
		return r;

	IConvHandle cd = iconv_open(UNICODE_CODEC, "utf-8");
	if (cd == (iconv_t)-1)
		return r;
	std::size_t iIn = str.size();
	const char *sIn = str.c_str();
	std::size_t iOut = iIn * 2;
	std::size_t iOutOrig = iOut;
	r.resize(iOut);
	char *buf = reinterpret_cast<char*>(&r[0]);
	std::size_t ret = iconv(cd, &sIn, &iIn, &buf, &iOut);
	if (ret == (std::size_t)-1) {
		r.clear();
		return r;
	}
	r.resize((iOutOrig - iOut) / 2);
	return r;
}

std::string ICONVext::unicodeToUtf8(const std::wstring &str)
{
	std::string r;
	if (str.empty())
		return r;

	IConvHandle cd = iconv_open("utf-8", UNICODE_CODEC);
	if (cd == (iconv_t)-1)
		return r;

	std::size_t iIn = str.size() * 2;
	const char *sIn = reinterpret_cast<const char*>(str.c_str());
	std::size_t iOut = iIn * 2;
	std::size_t iCoutOrig = iOut;
	r.resize(iOut);
	char *buf = reinterpret_cast<char*>(&r[0]);
	std::size_t ret = iconv(cd, &sIn, &iIn, &buf, &iOut);
	if (ret == (std::size_t) - 1) {
		r.clear();
		return r;
	}
	r.resize(iCoutOrig - iOut);

	return r;
}

std::string ICONVext::utf8ToMbcs(const std::string &str)
{
	std::string r;

	if (str.empty())
		return r;

	IConvHandle cd = iconv_open("gb2312", "utf-8");
	if (cd == (iconv_t)-1)
		return r;

	std::size_t iIn = str.size();
	std::size_t iOut = iIn * 2;
	std::size_t iOutOrig = iOut;
	const char *sIn = str.c_str();
	r.resize(iOut);
	char *buf = &r[0];
	size_t ret = iconv(cd, &sIn, &iIn, &buf, &iOut);
	if (ret == (std::size_t)-1) {
		return str;
	}
	r.resize(iOutOrig - iOut);
	return r;
}

std::string ICONVext::mbcsToUtf8(const std::string &str)
{
	std::string r;

	if (str.empty())
		return r;

	IConvHandle cd = iconv_open("utf-8", "gb2312");
	if (cd == (iconv_t)-1)
		return r;

	std::size_t iIn = str.size();
	std::size_t iOut = iIn * 2;
	std::size_t iOutOrig = iOut;
	const char *sIn = str.c_str();
	r.resize(iOut);
	char *buf = &r[0];
	size_t ret = iconv(cd, &sIn, &iIn, &buf, &iOut);
	if (ret == (std::size_t) - 1) {
		return str;
	}
	r.resize(iOutOrig - iOut);
	return r;
}

std::wstring ICONVext::mbcsToUnicode(const std::string &str)
{
	std::wstring r;
	IConvHandle cd = iconv_open(UNICODE_CODEC, "gb2312");
	if (cd == iconv_t(-1))
		return r;
	std::size_t iIn = str.size();
	std::size_t iOut = iIn * 2;
	std::size_t iOutOrg = iOut;
	const char *sIn = str.c_str();

	r.resize(iIn);
	char *buf = reinterpret_cast<char*>(&r[0]);
	std::size_t ret = iconv(cd, &sIn, &iIn, &buf, &iOut);
	if (ret == std::size_t(-1))
		return r;
	r.resize((iOutOrg - iOut) / sizeof(wchar_t));
	return r;
}

std::string ICONVext::unicodeToMbcs(const std::wstring &str)
{
	std::string r;
	IConvHandle cd = iconv_open("gb2312", UNICODE_CODEC);
	if (cd == iconv_t(-1))
		return r;
	std::size_t iIn = str.size() * 2;
	std::size_t iOut = iIn;
	std::size_t iOutOrig = iOut;
	const char *sIn = reinterpret_cast<const char*>(str.c_str());
	r.resize(iIn);
	char *buf = &r[0];
	std::size_t sRet = iconv(cd, &sIn, &iIn, &buf, &iOut);
	if (sRet == std::size_t(-1)) {
		r.resize(0);
		return r;
	}
	r.resize(iOutOrig - iOut);
	return r;
}

} // notstd
