#ifndef _BASELIB_CHARCONV_H
#define _BASELIB_CHARCONV_H

#include "config.h"
#include "iconv.h"

namespace notstd {

class IConvHandle {
private:
	iconv_t m_cd;

public:
	IConvHandle() : m_cd((iconv_t)-1)
	{
	}

	operator iconv_t() {
		return  m_cd;
	}

	~IConvHandle()
	{
		Close();
	}

	void Close()
	{
		if (m_cd != (iconv_t)-1)
		{
			iconv_close(m_cd);
			m_cd = (iconv_t)-1;
		}
	}

	IConvHandle(iconv_t cd)
		: m_cd((iconv_t)-1)
	{
		*this = cd;
	}

	IConvHandle& operator=(iconv_t cd)
	{
		Close();
		m_cd = cd;
		return *this;
	}
};

class NOTSTD_API ICONVext
{
public:
	static std::wstring utf8ToUnicode(const std::string &str);
	static std::string unicodeToUtf8(const std::wstring &str);
	static std::string mbcsToUtf8(const std::string &str);
	static std::string utf8ToMbcs(const std::string &str);
	static std::wstring mbcsToUnicode(const std::string &str);
	static std::string unicodeToMbcs(const std::wstring &str);
};

} // notstd

#endif
