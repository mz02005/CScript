#pragma once
#include "httpInterface.h"
#include "dataBufferImpl.h"
#include <unordered_map>

class Connection;
class HttpRequest;

class HttpResponse : public IHttpResponse
{
	friend class Connection;

private:
	HttpRequest *mHttpRequest;

	int mStatusCode;
	HttpDataBuffer *mDataBuffer;

	typedef std::unordered_map<std::string, std::string> ResponseHeaderList;
	ResponseHeaderList mResponseHeaderList;
	ResponseHeaderList::iterator mResponseHeaderPos;

public:
	HttpResponse();
	virtual ~HttpResponse();

	virtual int SetStatusCode(int code);
	virtual int GetStatusCode();

	// Header
	virtual int AppendHeader(const char *header, const char *val);
	virtual int SetHeader(const char *header, const char *val);

	virtual int GetHeader(const char *name,
		char *buffer, size_t *bufSize);
	virtual int GetFirstHeader();
	virtual int GetNextHeader(char *name, size_t *nameSize, char *val, size_t *valSize);
	virtual int ClearAllHeader();

	// Resp
	virtual IDataBuffer* GetResponseBuffer();
	virtual IHttpRequest* GetHttpRequest();

	void ClearAll();
};
