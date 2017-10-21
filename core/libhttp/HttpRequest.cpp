#include "stdafx.h"
#include "HttpRequest.h"
#include "httpServerNew.h"
#include "coder.h"
#include "notstd/notstd.h"

HttpRequest::HttpRequest()
{
	mDataBuffer = CreateRefObject<HttpDataBuffer>();
}

HttpRequest::~HttpRequest()
{
	if (mDataBuffer)
		mDataBuffer->DecRef();
}

void HttpRequest::ClearAll()
{
	mMethod.clear();
	mUrl.clear();
	mHttpVersion.clear();
	mRequestHeaderList.clear();
	mRequestHeaderPos = mRequestHeaderList.begin();
	mRequestParamList.clear();
	mRequestParamPos = mRequestParamList.begin();
}

bool HttpRequest::ParseParam()
{
	auto f = mUrl.find('?');
	if (f != mUrl.npos)
	{
		auto singleParam = notstd::StringHelper::SplitString(std::string(mUrl.c_str() + f + 1), "&");
		for (auto iter = singleParam.begin(); iter != singleParam.end(); iter++)
		{
			auto param = notstd::StringHelper::SplitString(*iter, "=");
			mRequestParamList.insert(std::make_pair(param[0], param.size() > 1 ? param[1] : ""));
		}
		mUrl.resize(f);
	}
	return true;
}

IDataBuffer* HttpRequest::GetReceiveBuffer()
{
	return mDataBuffer;
}

int HttpRequest::GetHttpRequestBaseInfo(HttpRequestBaseInfo *info)
{
	info->verb = mVerbose;
	pi_ncpy_s(info->url, sizeof(info->url), mUrl.c_str(), _TRUNCATE);
	pi_ncpy_s(info->version, sizeof(info->version), mHttpVersion.c_str(), _TRUNCATE);
	info->resLocalPosition = mResourceLocalPosition.c_str();
	info->posStringLen = mResourceLocalPosition.size();
	return 0;
}

int HttpRequest::SetResourceLocalPosition(const char *res, size_t len)
{
	if (!len)
		len = strlen(res);
	mResourceLocalPosition.assign(res, res + len);
	return 0;
}

int HttpRequest::AppendHeader(const char *header, const char *val)
{
	if (mRequestHeaderList.find(header) != mRequestHeaderList.end())
		return -HttpConst::E_AllreadyExists;
	mRequestHeaderList.insert(std::make_pair(header, val));
	return 0;
}

int HttpRequest::SetHeader(const char *header, const char *val)
{
	auto f = mRequestHeaderList.find(header);
	if (f == mRequestHeaderList.end())
	{
		mRequestHeaderList.insert(std::make_pair(header, val));
		return HttpConst::E_AllreadyExists;
	}
	f->second = val;
	return 0;
}

int HttpRequest::GetHeader(const char *name, char *buffer, size_t *bufSize)
{
	auto iter = mRequestHeaderList.find(name);
	if (iter != mRequestHeaderList.end())
	{
		return payHttpServer::OnGetStringBuffer(iter->second, buffer, bufSize);
	}

	return -HttpConst::E_Generic;
}

int HttpRequest::GetFirstHeader()
{
	mRequestHeaderPos = mRequestHeaderList.begin();
	return 0;
}

int HttpRequest::GetNextHeader(char *name, size_t *nameSize, char *val, size_t *valSize)
{
	if (mRequestHeaderPos != mRequestHeaderList.end())
	{
		int r1 = payHttpServer::OnGetStringBuffer(mRequestHeaderPos->first, name, nameSize);
		if (r1 >= 0)
		{
			int r2 = payHttpServer::OnGetStringBuffer(mRequestHeaderPos->second, val, valSize);
			mRequestHeaderPos++;
			return r2;
		}
		return r1;
	}
	return -HttpConst::E_Generic;
}

int HttpRequest::ClearAllHeader()
{
	mRequestHeaderList.clear();
	mRequestHeaderPos = mRequestHeaderList.begin();
	return 0;
}

int HttpRequest::GetParam(const char *name, char *buffer, size_t *bufSize)
{
	auto iter = mRequestParamList.find(name);
	if (iter != mRequestParamList.end())
	{
		return payHttpServer::OnGetStringBuffer(iter->second, buffer, bufSize);
	}
	return -HttpConst::E_Generic;
}

int HttpRequest::GetFirstParam()
{
	mRequestParamPos = mRequestParamList.begin();
	return 0;
}

int HttpRequest::GetNextParam(char *name, size_t *nameSize, char *val, size_t *valSize)
{
	if (mRequestParamPos != mRequestParamList.end())
	{
		int r1 = payHttpServer::OnGetStringBuffer(mRequestParamPos->first, name, nameSize);
		if (r1 >= 0)
		{
			int r2 = payHttpServer::OnGetStringBuffer(mRequestParamPos->second, val, valSize);
			mRequestParamPos++;
			return r2;
		}
		return r1;
	}
	return -HttpConst::E_Generic;
}

int HttpRequest::DoBasicAuthenticate(const char *username, const char *password)
{
	auto iter = mRequestHeaderList.find("authorization");
	if (iter == mRequestHeaderList.end())
		return -1;
	auto val = iter->second;

	static const char pre[] = "basic ";
	static const size_t preLen = strlen(pre);
	if (val.size() <= preLen)
		return -1;
	if (notstd::StringHelper::ToLower(val.substr(0, preLen)) != pre)
		return -1;

	std::string ss = username;
	ss.append(":").append(password);
	ss = Base64Encode(ss.c_str(), ss.size());
	if (ss == val.substr(preLen))
		return 0;
	return -1;
}
