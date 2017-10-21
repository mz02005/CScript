#pragma once

#include "dataBufferImpl.h"
#include <unordered_map>

class Connection;

class HttpRequest : public IHttpRequest
{
	friend class Connection;

private:
	Connection *mConn;

	HttpConst::Verb mVerbose;

	std::string mMethod;
	std::string mUrl;
	std::string mHttpVersion;

	std::string mResourceLocalPosition;

	typedef std::unordered_map<std::string, std::string> RequestHeaderList;
	RequestHeaderList mRequestHeaderList;

	typedef std::unordered_map<std::string, std::string> RequestParamList;
	RequestParamList mRequestParamList;

	RequestHeaderList::iterator mRequestHeaderPos;
	RequestParamList::iterator mRequestParamPos;

	HttpDataBuffer *mDataBuffer;

	void ClearAll();
	bool ParseParam();

public:
	HttpRequest();
	virtual ~HttpRequest();

	void SetConnection(Connection *conn) { mConn = conn; }

	// 
	virtual IDataBuffer* GetReceiveBuffer();
	virtual int GetHttpRequestBaseInfo(HttpRequestBaseInfo *info);
	virtual int SetResourceLocalPosition(const char *res, size_t len);

	// header
	virtual int AppendHeader(const char *header, const char *val);
	virtual int SetHeader(const char *header, const char *val);
	virtual int GetHeader(const char *name, char *buffer, size_t *bufSize);
	virtual int GetFirstHeader();
	virtual int GetNextHeader(char *name, size_t *nameSize, char *val, size_t *valSize);
	virtual int ClearAllHeader();

	// Param
	virtual int GetParam(const char *name, char *buffer, size_t *bufSize);
	virtual int GetFirstParam();
	virtual int GetNextParam(char *name, size_t *nameSize, char *val, size_t *valSize);

	virtual int DoBasicAuthenticate(const char *username, const char *password);
};
