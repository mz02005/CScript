#pragma once
#include "notstd/notstd.h"
#include "core/libhttp/httpInterface.h"
#include <string>
#include <unordered_map>
#include <tuple>

#ifdef HMHTTPTEST_EXPORTS
#define HMHTTPTEST_API __declspec(dllexport)
#else
#define HMHTTPTEST_API __declspec(dllimport)
#endif

template <typename T>
class Singleton
{
public:
	static T& GetInstance() {
		static T t;
		return t;
	}
};

class MimeManager
{
private:
	std::unordered_map<std::string, std::string> mMimeTypeTable;

private:
	int LoadMimeConfInner(const std::string &filePathName);

public:
	int LoadMimeConf();
	std::string GetMimeType(const std::string &extName) const;
};

class UrlHelper
{
public:
	static inline unsigned char ToHex2(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	static inline unsigned char FromHex2(unsigned char x)
	{
		unsigned char y = 0;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		return y;
	}

	static inline std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex2(static_cast<unsigned char>(str[i]) >> 4);
				strTemp += ToHex2(static_cast<unsigned char>(str[i]) % 16);
			}
		}
		return strTemp;
	}

	static inline std::string UrlDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				//				assert(i + 2 < length);  
				unsigned char high = FromHex2((unsigned char)str[++i]);
				unsigned char low = FromHex2((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}
};

extern "C" {
	uint64_t GetModuleCapability();
	StepResult DoStep(void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s);
}

// hmhttptest的配置文件是xml文件，下面是其大概的样子
//<?xml version=\"1.0\" encoding=\"utf-8\"?>
//<httptest>
//	<filesystem>
//		<basicauth username=\"guest\" password=\"iamabadbear\" />
//		<network sendBufSize=\"16384\" />
//	</filesystem>
//	<releaseList>
//		<add local=\"c:/\" url=\"/\"0 username=\"test1\" password=\"test1\" />
//		<add local=\"c:/users/\" url=\"/users\" username=\"test2\" password=\"test2\" />
//	</releaseList>
//</httptest>

class add : public notstd::XmlElemSerializerBase
{
	DECLARE_XMLSERIAL_ELEM(add)

public:
	notstd::stringBaseTypeXmlProp mLocal, mUrl;
	notstd::stringBaseTypeXmlProp mUsername, mPassword;
	notstd::stringBaseTypeXmlProp mExtWhiteList, mExtBlackList;

	std::vector<std::string> mRealExtWhiteList;
	std::vector<std::string> mRealExtBlackList;

	virtual void Normalize(XmlElemSerializerBase *parent) override;
};

class releaseList : public notstd::XmlElemSerializerBase
{
	DECLARE_XMLSERIAL_ELEM(releaseList)

public:
	notstd::SubElementList mAddList;
};

class basicauth : public notstd::XmlElemSerializerBase
{
	DECLARE_XMLSERIAL_ELEM(basicauth)

public:
	basicauth();

	notstd::stringBaseTypeXmlProp mUsername, mPassword;
};

class network : public notstd::XmlElemSerializerBase
{
	DECLARE_XMLSERIAL_ELEM(network)

public:
	network();
	notstd::uint32_tBaseTypeXmlProp mSendBufSize;
};

class filesystem : public notstd::XmlElemSerializerBase
{
	DECLARE_XMLSERIAL_ELEM(filesystem)

public:
	basicauth mBasicAuth;
	network mNetwork;
};

class httptest : public notstd::XmlElemSerializerBase
{
	DECLARE_XMLSERIAL_ELEM(httptest)

public:
	filesystem mFileSystem;
	releaseList mReleaseList;
};

////////////////////////////////////////////////////////////////////////////////

class PathIndex
{
private:
	typedef std::vector<std::pair<std::string, std::tuple<std::string, std::string, std::string, std::vector<std::string>*, std::vector<std::string>*>>> PathTable;
	PathTable mPathTable;

public:
	int LoadTableFromHttpTest(httptest &httpTest);
	int GetPathIndex(const std::string &url) const;

	const PathTable& GetPathTable() const { return mPathTable; }
};
