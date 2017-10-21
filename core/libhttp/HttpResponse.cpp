#include "stdafx.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "httpServerNew.h"

HttpResponse::HttpResponse()
	: mStatusCode(500)
{
	mDataBuffer = CreateRefObject<HttpDataBuffer>();
}

HttpResponse::~HttpResponse()
{
	if (mDataBuffer)
		mDataBuffer->DecRef();
}

int HttpResponse::SetStatusCode(int code)
{
	mStatusCode = code;
	return 0;
}

int HttpResponse::GetStatusCode()
{
	return mStatusCode;
}

int HttpResponse::AppendHeader(const char *header, const char *val)
{
	if (mResponseHeaderList.find(header) != mResponseHeaderList.end())
		return -HttpConst::E_AllreadyExists;
	mResponseHeaderList.insert(std::make_pair(header, val));
	return 0;
}

int HttpResponse::SetHeader(const char *header, const char *val)
{
	auto f = mResponseHeaderList.find(header);
	if (f == mResponseHeaderList.end())
	{
		mResponseHeaderList.insert(std::make_pair(header, val));
		return HttpConst::E_AllreadyExists;
	}
	f->second = val;
	return 0;
}

int HttpResponse::GetHeader(const char *name, char *buffer, size_t *bufSize)
{
	auto iter = mResponseHeaderList.find(name);
	if (iter != mResponseHeaderList.end())
	{
		return payHttpServer::OnGetStringBuffer(iter->second, buffer, bufSize);
	}

	return -HttpConst::E_Generic;
}

int HttpResponse::GetFirstHeader()
{
	mResponseHeaderPos = mResponseHeaderList.begin();
	return 0;
}

int HttpResponse::GetNextHeader(char *name, size_t *nameSize, char *val, size_t *valSize)
{
	if (mResponseHeaderPos != mResponseHeaderList.end())
	{
		int r1 = payHttpServer::OnGetStringBuffer(mResponseHeaderPos->first, name, nameSize);
		if (r1 >= 0)
		{
			int r2 = payHttpServer::OnGetStringBuffer(mResponseHeaderPos->second, val, valSize);
			mResponseHeaderPos++;
			return r2;
		}
		return r1;
	}
	return -HttpConst::E_Generic;
}

int HttpResponse::ClearAllHeader()
{
	mResponseHeaderList.clear();
	mResponseHeaderPos = mResponseHeaderList.begin();
	return 0;
}

IDataBuffer* HttpResponse::GetResponseBuffer()
{
	return mDataBuffer;
}

IHttpRequest* HttpResponse::GetHttpRequest()
{
	return static_cast<IHttpRequest*>(mHttpRequest);
}

void HttpResponse::ClearAll()
{
	mResponseHeaderList.clear();
	mResponseHeaderPos = mResponseHeaderList.begin();
	mDataBuffer->RemoveAll();
}
