/*
Add by mz02005
*/

#include "stdafx.h"
#include <regex>
#include <sstream>
#include "httpsdef.h"
#include "notstd/notstd.h"

#ifdef PLATFORM_WINDOWS
#include <WS2tcpip.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif

inline unsigned char ToHex2(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

inline unsigned char FromHex2(unsigned char x)
{
	unsigned char y = 0;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	return y;
}

inline std::string UrlEncode(const std::string& str)
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

inline std::string UrlDecode(const std::string& str)
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

///////////////////////////////////////////////////////////////////////////////
// 打印formdata
void FormDataHelper::WriteToString(const FormData &fd, std::string &s)
{
	std::stringstream ss;
	ss << "<ul>";
	for (auto iter = fd.begin(); iter != fd.end(); iter++)
		ss << "<li>" << iter->first << ": " << iter->second << "</li>\r\n";
	ss << "</ul>";
	s = ss.str();
}

///////////////////////////////////////////////////////////////////////////////
// 为了测试，这里添加一个缺省的带有表单的界面，便于用浏览器测试
static const char *defPage = R"(<html><head><title>%%title%%</title></head><body>
<form action="/test" method="POST">
<p><input type="text" name="username" value="username" /></p>
<p><input type="text" name="sex" value="sex" /></p>
<p><input type="text" name="age" value="age" /></p>
<p><input type="text" name="weight" value="weight" /></p>
<p><input type="submit" value="递交" /></p>
<p></form></body></html>)";

///////////////////////////////////////////////////////////////////////////////

int payHttpServer::OnGetStringBuffer(const std::string &str, char *&buffer, size_t *&bufSize)
{
	if (!bufSize || !*bufSize)
		return -HttpConst::E_ParamError;

	if (str.size() >= *bufSize)
		return -HttpConst::E_NotEnoughBufferSize;

	memcpy(buffer, str.c_str(), str.size() + 1);
	*bufSize = str.size();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

Connection::Connection()
	: mDel(false)
	, mKeepAlive(false)
	, mBufferEvent(nullptr)
	, mHttpRequest(nullptr)
	, mHttpResponse(nullptr)
	, mLastFind(0)
	, mHttpStage(HttpStage::CS_WAIT_METHOD)
	, mDataLength(-1)
{
	mLastSend = notstd::GetCurrentTick();
	mLastRecv = mLastSend;

	mHttpRequest = CreateRefObject<HttpRequest>();
	mHttpRequest->SetConnection(this);

	mHttpResponse = CreateRefObject<HttpResponse>();
	mHttpResponse->mHttpRequest = mHttpRequest;
}

Connection::~Connection()
{
	DestroyData();
	if (mHttpRequest)
		mHttpRequest->DecRef();
	if (mHttpResponse)
		mHttpResponse->DecRef();
}

int Connection::CreateSocketBuffer(event_base *eb)
{
	mBufferEvent = bufferevent_socket_new(eb, mSock, BEV_OPT_CLOSE_ON_FREE);
	return 0;
}

int Connection::ConnSendData(const char *d, size_t l)
{
	return bufferevent_write(mBufferEvent, d, l);
}

void Connection::ConnOnRecv(const char *d, size_t l)
{
	OnRecv(reinterpret_cast<const void*>(d), l);
}

void Connection::DestroyData()
{
	for (auto iter = mUserdataTable.begin(); iter != mUserdataTable.end(); iter++)
	{
		auto &ud = iter->second.userdata;
		auto onDestroy = iter->second.onDestroy;
		if (onDestroy)
			(*onDestroy)(ud);
		::free(ud);
	}
	mHttpRequest->GetReceiveBuffer()->RemoveAll();
	mHttpResponse->GetResponseBuffer()->RemoveAll();
	mUserdataTable.clear();
}

void Connection::SetBaseInfo(payHttpServer *server, SOCKET sock)
{
	mServer = server;
	mSock = sock;
}

void* Connection::CreateData(void *ptr, size_t s, ConnectionDataDestoy onDestroy)
{
	UserdataEntry ue;
	auto iter = mUserdataTable.find(ptr);
	if (iter == mUserdataTable.end())
	{
		ue.ptr = ptr;
		ue.userdata = s ? ::malloc(s) : nullptr;
		if (ue.userdata)
		{
			memset(ue.userdata, 0, s);
		}
		ue.onDestroy = onDestroy;
		mUserdataTable.insert(std::make_pair(ptr, ue));
		return ue.userdata;
	}
	return iter->second.userdata;
}

void* Connection::FindData(void *ptr)
{
	auto iter = mUserdataTable.find(ptr);
	if (iter != mUserdataTable.end())
	{
		return iter->second.userdata;
	}
	return nullptr;
}

int Connection::GoNextStage(HttpStage &stage)
{
	auto x = static_cast<int>(stage);
	stage = (HttpStage)(x + 1);
	return 0;
}

StepResult Connection::SendSimpleTextRespond(int statusCode, const void *data, size_t s)
{
	mHttpResponse->ClearAll();

	size_t len = s == -1 ? strlen(reinterpret_cast<const char*>(data)) : s;

	mHttpResponse->SetStatusCode(statusCode);
	mHttpResponse->AppendHeader("server", "days http server(by mz02005)");
	mHttpResponse->AppendHeader("content-length", std::to_string(len).c_str());
	mHttpResponse->AppendHeader("content-type", "text/plain");

	auto buf = mHttpResponse->GetResponseBuffer();
	auto piece = buf->CreatePeice();
	piece->AppendData(data, len);
	buf->AppendPiece(piece);

	mHttpStage = HttpStage::CS_SEND_Header;

	return NTD_COMPLETE;
}

StepResult Connection::SendSimpleTextRespond(int statusCode, const char *data)
{
	return SendSimpleTextRespond(statusCode, data, strlen(data));
}

StepResult Connection::SendRespond(int statusCode, const char *contentType, const char *data, size_t s,
	int(*OnBeforeSendResponse)(IConnection *conn, IHttpRequest *request, IHttpResponse *resp))
{
	mHttpResponse->ClearAll();

	mHttpResponse->SetStatusCode(statusCode);
	mHttpResponse->AppendHeader("server", "days http server(by mz02005)");
	size_t v = (s == -1 ? strlen(data) : s);
	mHttpResponse->AppendHeader("content-length", std::to_string(v).c_str());
	mHttpResponse->AppendHeader("content-type", contentType);

	auto buf = mHttpResponse->GetResponseBuffer();
	auto piece = buf->CreatePeice();
	piece->AppendData(data, v);
	buf->AppendPiece(piece);

	if (OnBeforeSendResponse)
	{
		if ((*OnBeforeSendResponse)(this, mHttpRequest, mHttpResponse) < 0)
			return NTD_ERROROCCURED;
	}

	mHttpStage = HttpStage::CS_SEND_Header;

	return NTD_COMPLETE;
}

StepResult Connection::Send401Respond(const char *realm)
{
	mHttpResponse->ClearAll();

	mHttpResponse->SetStatusCode(401);
	mHttpResponse->AppendHeader("server", "days http server(by mz02005)");
	mHttpResponse->AppendHeader("content-length", "0");
	mHttpResponse->AppendHeader("content-type", "text/plain");
	mHttpResponse->AppendHeader("WWW-authenticate", std::string("Basic realm=\"").append(realm).append("\"").c_str());

	mHttpStage = HttpStage::CS_SEND_Header;

	return NTD_COMPLETE;
}

// 解析http请求第一行的函数
bool Connection::ParseHttpMethod(const char *s, size_t l)
{
	std::cmatch mr;
	std::regex httpMethodRegex("(\\w+)\\s+([^\\s]+)\\s+(HTTP/1\\.[01])");
	if (!std::regex_match(s, s + l, mr, httpMethodRegex))
		return false;
	mHttpRequest->mMethod.assign(mr[1].first, mr[1].second);

	static const struct MethodEntry
	{
		const char *method;
		HttpConst::Verb verb;
	} entries[] =
	{
		{ "GET", HttpConst::V_GET, },
		{ "POST", HttpConst::V_POST, },
		{ "PUT", HttpConst::V_PUT, },
		{ "HEAD", HttpConst::V_HEAD, },
		{ "COPY", HttpConst::V_COPY, },
		{ "MOVE", HttpConst::V_MOVE, },
	};

	static const MethodEntry entriesLong[]
	{
		{ "OPTIONS", HttpConst::V_OPTIONS, },
		{ "DELETE", HttpConst::V_DELETE, },
		{ "TRACK", HttpConst::V_TRACK, },
		{ "PROPFIND", HttpConst::V_PROPFIND, },
		{ "PROPPATCH", HttpConst::V_PROPPATCH, },
		{ "MKCOL", HttpConst::V_MKCOL, },
		{ "SEARCH", HttpConst::V_SEARCH, },
	};

	const MethodEntry *e;
	const MethodEntry *last;

	auto ms = mHttpRequest->mMethod.size();
	if (ms < 3 || ms > 4)
	{
		last = entriesLong + sizeof(entriesLong) / sizeof(entriesLong[0]);
		e = entriesLong;

		for (; e != last; e++)
		{
			if (mHttpRequest->mMethod == e->method)
			{
				mHttpRequest->mVerbose = e->verb;
				break;
			}
		}
	}
	else
	{
		last = entries + sizeof(entries) / sizeof(entries[0]);
		e = entries;

		for (; e != last; e++)
		{
			if (*reinterpret_cast<const int*>(e->method) ==
				*reinterpret_cast<const int*>(mHttpRequest->mMethod.c_str()))
			{
				mHttpRequest->mVerbose = e->verb;
				break;
			}
		}
	}

	if (e == last)
		return false;

	//std::regex urlRegex(
	//	"/([^\\?]*)"
	//	"(\\?([^\\=]+\\=[^&]*&)*([^\\=]+\\=[^&]*))?");
	//std::smatch sm;
	mHttpRequest->mUrl.assign(mr[2].first, mr[2].second);
	if (!mHttpRequest->ParseParam())
		return false;
	//if (!std::regex_match(mHttpRequest->mUrl, sm, urlRegex))
	//	return false;

	// 本服务器仅支持HTTP1.1，在后面的处理中会检查这个
	mHttpRequest->mHttpVersion.assign(mr[3].first, mr[3].second);

	return true;
}

// 解析一行请求头
bool Connection::ParseSingleHeader(const char *s, size_t l)
{
	const char *p = std::find(s, s + l, ':');

	std::pair<std::string, std::string> data;
	if (p == s + l)
		data = std::make_pair(std::string(s, s + l), "");
	else
		data = std::make_pair(std::string(s, p), std::string(p + 1, s + l));

	std::transform(data.first.begin(), data.first.end(), data.first.begin(), &tolower);
	//std::transform(data.second.begin(), data.second.end(), data.second.begin(), &tolower);

	// 返回请求中可能含有空格
	data.second.erase(0, data.second.find_first_not_of(" \t"));
	data.second.erase(data.second.find_last_not_of(" \t") + 1);

	mHttpRequest->mRequestHeaderList.insert(data);
	return true;
}

StepResult Connection::SendResponse(int statusCode, const std::string &s, const std::string &contentType)
{
	mHttpResponse->SetStatusCode(statusCode);
	mHttpResponse->AppendHeader("server", "days http server(by mz02005)");
	mHttpResponse->AppendHeader("content-length", std::to_string(s.size()).c_str());
	if (!contentType.empty())
		mHttpResponse->AppendHeader("content-type", contentType.c_str());

	auto buf = mHttpResponse->GetResponseBuffer();
	auto piece = buf->CreatePeice();
	piece->AppendData(s.c_str(), s.size());
	buf->AppendPiece(piece);

	mHttpStage = HttpStage::CS_SEND_Header;

	return NTD_COMPLETE;
}

//// 解析表单数据
//bool Connection::ParseFormData(FormData &formData)
//{
//	// 通过'&'分割
//	logout::StringHelper::StringArray sa = logout::StringHelper::SplitString(mDataBuff, "&");
//	for (decltype(sa.size()) i = 0; i < sa.size(); i++)
//	{
//		// 通过'='分割
//		auto f = sa[i].find('=');
//		formData.insert(
//			std::make_pair(f != std::string::npos ? sa[i].substr(0, f) : sa[i],
//			f != std::string::npos ? sa[i].substr(f + 1) : "")
//			);
//	}
//	return true;
//}

void Connection::OnSend(bufferevent *bev, void *arg)
{
	reinterpret_cast<Connection*>(arg)->OnSend();
}

void Connection::OnRecv(bufferevent *bev, void *arg)
{
	size_t s;
	char buf[8192];
	evutil_socket_t sock = bufferevent_getfd(bev);

	auto connBase = static_cast<Connection*>(arg);
	connBase->mLastRecv = notstd::GetCurrentTick();
	while (s = bufferevent_read(bev, buf, sizeof(buf) - 1), s > 0)
	{
		buf[s] = 0;
		connBase->ConnOnRecv(buf, s);
		//connBase->OnRecv(buf, s);
	}
}

void Connection::OnEvent(bufferevent *bev, short event, void *arg)
{
	auto conn = static_cast<Connection*>(arg);
	if (event == BEV_EVENT_CONNECTED)
		return;
	conn->SetDelete();
}

void Connection::OnSend()
{
	mLastSend = notstd::GetCurrentTick();
	StepFireCond cond = StepFireCond::SFC_WRITE;
	if (mServer->DoStep(this, cond, mHttpStage, mHttpRequest, mHttpResponse, nullptr, 0) < 0)
		SetDelete();
}

void Connection::OnMaintenance(DWORD tick)
{
	if (mDel)
		return;
	StepFireCond cond = StepFireCond::SFC_ASYNC;
	if (mServer->DoStep(this, cond, mHttpStage, mHttpRequest, mHttpResponse, nullptr, 0) < 0)
		SetDelete();
	if (!mDel && tick - mLastRecv >= CONNECT_IDLE_TIME)
		SetDelete();
}

StepResult Connection::DoPositionResource(void *userData, IConnection *param, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	return static_cast<Connection*>(param)->DoPositionResource(cond, stage, request, resp, buf, s);
}

StepResult Connection::DoPositionResource(StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	if (StepFireCond::SFC_READ == cond)
	{
		HttpRequestBaseInfo baseInfo;
		if (request->GetHttpRequestBaseInfo(&baseInfo) < 0)
			return NTD_ERROROCCURED;

		if (strcmp(baseInfo.url, "/") == 0)
		{
			if (baseInfo.verb == HttpConst::V_GET)
			{
				IncStage(stage);
				return NTD_COMPLETE;
			}
			else
				return SendResponse(403, "Only GET supported for the resource", "text/plain");
		}

		return NTD_CONTINUE;
	}
	return NTD_ERROROCCURED;
}

StepResult Connection::DefaultStep(StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	if (StepFireCond::SFC_READ == cond
		|| StepFireCond::SFC_OTHER == cond)
	{
		HttpRequestBaseInfo baseInfo;
		if (request->GetHttpRequestBaseInfo(&baseInfo) < 0)
			return NTD_ERROROCCURED;

		if (stage >= CS_DECODE && stage <= CS_AUTHORITY)
		{
			// 只允许http1.1版本的请求继续处理
			// 由于在之前处理请求头的模块已经使用正则表达式来限制了版本字符，所以，这里最后一个字符必定是1或者0
			if (static_cast<HttpRequest*>(request)->mHttpVersion.back() != '1')
				return SendResponse(505, "Only HTTP1.1 supported");

			if (stage == CS_POSITION_RESOURCE)
			{
				if (strcmp(baseInfo.url, "/") == 0)
				{
					if (baseInfo.verb != HttpConst::V_GET)
					{
						return SendResponse(403, "Only GET supported for the resource", "text/plain");
					}

					IncStage(stage);
					return NTD_COMPLETE;
				}
				return NTD_CONTINUE;
			}
			//else if (stage == CS_AUTHORITY)
			//{
			//	if (request->DoBasicAuthenticate("aaa", "aaa") < 0)
			//		return Send401Respond("need basic auth");
			//}
			IncStage(stage);
			return NTD_COMPLETE;
		}
		else if (CS_GENERATE_RESPONSE_DATA == stage)
		{
			if (strcmp(baseInfo.url, "/") == 0)
				return SendResponse(200, "Welcome to my site", "text/plain");
			return NTD_CONTINUE;
		}
		else if (CS_SEND_Header == stage)
		{
			std::string temp, total;
			total = notstd::StringHelper::Format("HTTP/1.1 %d notused\r\n", mHttpResponse->GetStatusCode());

			int rr = mHttpResponse->GetFirstHeader();
			if (rr < 0)
				return NTD_ERROROCCURED;
			char name[512], val[512];
			size_t lenName = sizeof(name), lenVal = sizeof(val);
			while (rr = mHttpResponse->GetNextHeader(name, &lenName, val, &lenVal), rr >= 0)
			{
				total += notstd::StringHelper::Format("%s: %s\r\n", name, val);
				lenName = sizeof(name);
				lenVal = sizeof(val);
			}
			total += "\r\n";

			stage = HttpStage::CS_WAIT_HeaderSendOver;
			ConnSendData(total.c_str(), total.size());

			return NTD_COMPLETE;
		}
		else if (CS_SEND_Body == stage)
		{
			auto buf = mHttpResponse->GetResponseBuffer();
			if (buf->GetFirstPiece() < 0)
			{
				stage = HttpStage::CS_WAIT_METHOD;
				ResetState();
				cond = StepFireCond::SFC_READ;
				return NTD_COMPLETE;
			}
			IBufferPiece *piece = nullptr;
			void *theBuf;
			size_t bufSize;
			std::string toSend;
			while (buf->GetNextPiece(piece) >= 0)
			{
				if (piece->GetBuffer(theBuf, bufSize) < 0)
					return NTD_ERROROCCURED;
				if (bufSize)
					toSend.append(reinterpret_cast<const char*>(theBuf), bufSize);
			}

			if (toSend.size())
			{
				stage = HttpStage::CS_WAIT_BodySendOver;
				ConnSendData(toSend.c_str(), toSend.size());
			}
			else
			{
				stage = HttpStage::CS_WAIT_METHOD;
				ResetState();
				cond = StepFireCond::SFC_READ;
			}
			return NTD_COMPLETE;
		}
	}
	else if (StepFireCond::SFC_WRITE == cond)
	{
		bool callStepAgain = true;
		if (HttpStage::CS_WAIT_HeaderSendOver == stage)
		{
			// 注意，跳过了编码阶段，直接发送body缓冲
			stage = CS_SEND_Body;
			cond = StepFireCond::SFC_OTHER;
			return NTD_COMPLETE;
		}
		else if (HttpStage::CS_WAIT_BodySendOver == stage)
		{
			stage = HttpStage::CS_WAIT_FOR_NEWROUND;
			ResetState();
			cond = StepFireCond::SFC_READ;
			return NTD_COMPLETE;
		}
		else
			return NTD_ERROROCCURED;
	}
	return NTD_COMPLETE;
}

StepResult Connection::DoHeaderReceive(StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	assert(stage <= HttpStage::CS_WAIT_HEADER);

	if (cond != StepFireCond::SFC_READ)
		return NTD_COMPLETE;
			
	mDataBuff.append(reinterpret_cast<const char*>(buf), s);
	s = 0;

	if (HttpStage::CS_WAIT_METHOD == stage)
	{
		auto f = mDataBuff.find("\r\n", mLastFind);
		if (f == mDataBuff.npos)
		{
			mLastFind = mDataBuff.size();
			return NTD_COMPLETE;
		}
		stage = HttpStage::CS_WAIT_HEADER;
		if (!ParseHttpMethod(&mDataBuff[0], f))
			return NTD_ERROROCCURED;
		mDataBuff.erase(0, f + 2);
		mLastFind = 0;
	}
	else if (HttpStage::CS_WAIT_HEADER == stage)
	{
		auto f = mDataBuff.find("\r\n", mLastFind);
		while (f != mDataBuff.npos)
		{
			if (f == 0)
			{
				// 遇到空行，则表示头结束，等待接收body了

				// 根据http1.1协议，body的长度要么在头的Content-Length中标明
				// 或者等待链接中断表示数据传输结束（对于服务端不存在这种情况）
				// 如果在头中包含了Transfer-Encoding: chunk\r\n，则表示使用chunk方式进行传输
				// 传输将通过带有长度头的数据帧来进行
				// 由于本程序是一个简单的http服务器，所以，这里不支持Transfer-Encoding头，同时也不支持
				// 通过inflate、gzip等压缩后的数据

				char theBuf[1024];
				size_t bz = sizeof(theBuf);
				if (mHttpRequest->GetHeader("content-length", theBuf, &bz) >= 0)
				{
					std::stringstream ss;
					ss << theBuf;
					ss >> mDataLength;
					if (mDataLength < 0)
						return StepResult::NTD_ERROROCCURED;
				}
				else
				{
					mDataLength = 0;
				}
				{
					char connFlag[128];
					size_t connSize = sizeof(connFlag);
					if (mHttpRequest->GetHeader("connection", connFlag, &connSize) >= 0
						&& !strcmp(connFlag, "keep-alive"))
						mKeepAlive = true;
				}
				IncStage(stage);
				mDataBuff.erase(0, 2);
				return NTD_COMPLETE;
			}
			else
			{
				if (!ParseSingleHeader(mDataBuff.c_str(), f))
					return StepResult::NTD_ERROROCCURED;
				mDataBuff.erase(0, f + 2);
				mLastFind = 0;
			}
			f = mDataBuff.find("\r\n", mLastFind);
		}
		mLastFind = mDataBuff.size();
	}
		
	return NTD_CONTINUE;
}

StepResult Connection::DoHeaderReceive(void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	return static_cast<Connection*>(conn)->DoHeaderReceive(cond, stage, request, resp, buf, s);
}

StepResult Connection::DoReceiveBodyInner(void *userData, IConnection *param, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	return static_cast<Connection*>(param)->DoReceiveBodyInner(cond, stage, request, resp, buf, s);
}

StepResult Connection::DoReceiveBodyInner(StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	mDataBuff.append(reinterpret_cast<const char*>(buf), s);
	s = 0;

	assert(stage == HttpStage::CS_WAIT_BODY);
	if (mDataLength != (uint64_t)-1)
	{
		if (mDataBuff.size() >= mDataLength)
		{
			auto buffer = request->GetReceiveBuffer();
			auto piece = buffer->CreatePeice();
			piece->AppendData(mDataBuff.c_str(), static_cast<size_t>(mDataLength));
			buffer->AppendPiece(piece);
			mDataBuff.erase(0, static_cast<size_t>(mDataLength));
			mDataLength = -1;
			IncStage(stage);
		}
		else
			return NTD_COMPLETE;
	}
	return NTD_COMPLETE;
}

inline void Connection::IncStage(HttpStage &stage)
{
	GoNextStage(stage);
}

void Connection::ResetState()
{
	mHttpRequest->ClearAll();
	mHttpResponse->ClearAll();
}

void Connection::OnRecv(const void *buf, size_t s)
{
	mLastRecv = notstd::GetCurrentTick();
	StepFireCond cond = StepFireCond::SFC_READ;
	if (mServer->DoStep(this, cond, mHttpStage, mHttpRequest, mHttpResponse, buf, s) < 0)
		SetDelete();
}

bool Connection::InitializeConn(Connection *conn)
{
	conn->CreateSocketBuffer(conn->mServer->mEventBase);
	bufferevent_setcb(conn->mBufferEvent, &Connection::OnRecv,
		&Connection::OnSend, &Connection::OnEvent, conn);
	bufferevent_enable(conn->mBufferEvent, EV_READ | EV_WRITE | EV_ET);
	conn->mServer->AddConnection(conn);
	return true;
}

// 创建新的链接，并开始接收数据
Connection* Connection::NewConnection(payHttpServer *server, SOCKET sock)
{
	auto conn = CreateRefObject<Connection>();
	conn->SetBaseInfo(server, sock);
	if (!InitializeConn(conn))
	{
		delete conn;
		return nullptr;
	}
	return conn;
}

// 由维护线程调用，如果链接已经失效，就删除它对应的数据结构，
// 并通知维护线程在链接列表中删除自己
bool Connection::DestroyConnection(Connection *conn)
{
	bool r = conn->mDel;
	if (r) {
		conn->DecRef();
	}
	return r;
}

///////////////////////////////////////////////////////////////////////////////

payHttpServer::payHttpServer()
	: mStartupOK(false)
	//, mStopCleanupThread(false)
	, mEventBase(nullptr)
	, mMaintenanceTimer(nullptr)
	, mListenEvent(nullptr)
	, mListenSocket(INVALID_SOCKET)
	, mTimerCounter(0)
	, mSSLContext(nullptr)
{
	mConf.port = 80;

	mWorkflow.resize(HttpStage::CS_MAX);

	// windows环境下需要初始化一些数据结构（初始化winsock环境和获得一些
	// 高级函数的函数指针）
	//notstd::NetEnviroment::Init();
}

payHttpServer::~payHttpServer()
{
	StopPayHttpServer();
	//notstd::NetEnviroment::Term();
}

void payHttpServer::AddInnerExtModule(GetModuleCapabilityProc p1, DoStepProc p2)
{
	ModuleEntry me;
	me.doNotFreeModule = 1;
	me.module = nullptr;
	me.GetModuleCapability = p1;
	me.DoStep = p2;
	mModuleEntryList.emplace_back(me);
}

void payHttpServer::LoadExtModule()
{
	// 缺省的处理器
	AddInnerExtModule([]()->uint64_t { return (1 << HttpStage::CS_WAIT_HEADER) | (1 << HttpStage::CS_WAIT_METHOD); }, &Connection::DoHeaderReceive);
	AddInnerExtModule([]()->uint64_t { return (1 << HttpStage::CS_WAIT_BODY); }, &Connection::DoReceiveBodyInner);
	AddInnerExtModule([]()->uint64_t { return (1 << HttpStage::CS_POSITION_RESOURCE); }, &Connection::DoPositionResource);

	// 加载配置信息里传进来的
	if (mConf.entry.DoStep && mConf.entry.GetModuleCapability)
		AddInnerExtModule(mConf.entry.GetModuleCapability, mConf.entry.DoStep);

	std::string localPath = notstd::AppHelper::GetAppDir();

	//WIN32_FIND_DATAA fd;
	//std::string toFind = localPath + "*.dll";
#ifdef PLATFORM_WINDOWS
	static const char *ext = "dll";
#else
	static const char *ext = "so";
#endif

	notstd::CFindIterator fi(localPath);
	for (notstd::CFindResult fr = fi.begin(); fr != fi.end(); fr = fi.next())
	{
		if (fr.GetSuffix() != ext)
			continue;
		if (fr.GetName().substr(0, 2) != "hm")
			continue;

		ModuleEntry me;
		me.doNotFreeModule = 0;
		me.modulePathName = fr.GetPath();
		me.module = notstd::loadlibrary(me.modulePathName.c_str());
		if (me.module)
		{
			me.GetModuleCapability = (GetModuleCapabilityProc)notstd::getprocaddress(me.module, GETMODULECAPABILITY);
			me.DoStep = (DoStepProc)notstd::getprocaddress(me.module, DOSTEP);
			me.InitializeModule = (InitializeModuleProc)notstd::getprocaddress(me.module, INITIALIZEMODULE);
			me.UninitializeModule = (UninitializeModuleProc)notstd::getprocaddress(me.module, UNINITIALIZEMODULE);

			if (
				((!!me.InitializeModule) ^ (!!me.UninitializeModule))
				|| !me.GetModuleCapability || !me.DoStep
				|| (me.InitializeModule && (*me.InitializeModule)() < 0)
				)
			{
				notstd::freelibrary(me.module);
			}
			else
			{
				mModuleEntryList.emplace_back(me);
				printf("LoadLibrary succeeded: %s\n", me.modulePathName.c_str());
			}
		}
	}

	// 遍历处理表，进行处理过程的预分配
	for (auto &entry : mModuleEntryList)
	{
		auto cap = (*entry.GetModuleCapability)();
		for (int i = 0; i < 64; i++)
		{
			if ((cap >> i) & 0x1)
				mWorkflow[i].push_back(&entry);
		}
	}
}

void payHttpServer::UnloadExtModule()
{
	for (auto x : mModuleEntryList)
	{
		if (!x.doNotFreeModule)
			notstd::freelibrary(x.module);
	}
	mModuleEntryList.clear();
}

// 在创建链接对象时，需将创建的链接加入到维护列表中，由维护线程统一管理
void payHttpServer::AddConnection(Connection *conn)
{
	LockConnectionList();
	mHttpConnList.push_back(conn);
	UnlockConnectionList();
}

void payHttpServer::LockConnectionList()
{
}

void payHttpServer::UnlockConnectionList()
{
}

//void payHttpServer::OnCleanup()
//{
//	for (; !mStopCleanupThread;)
//	{
//		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//		LockConnectionList();
//		for (auto &iter = mHttpConnList.begin(); iter != mHttpConnList.end();)
//		{
//			if (Connection::DestroyConnection(*iter))
//			{
//				iter = mHttpConnList.erase(iter);
//			}
//			else
//				iter++;
//		}
//		UnlockConnectionList();
//	}
//}

int payHttpServer::DoStep(Connection *conn, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *response, const void *buf, size_t s)
{
	HttpStage stageOrig = stage;
	StepResult stepRet = NTD_ERROROCCURED;

	if (HttpStage::CS_WAIT_METHOD == stage && cond == StepFireCond::SFC_WRITE)
		return 0;

	if (stage == CS_WAIT_METHOD)
		conn->mKeepAlive = false;

	while (1)
	{
		if (stage >= static_cast<int>(mWorkflow.size()))
			return -1;

		if (CS_WAIT_METHOD == stage)
		{
			conn->DestroyData();
		}

		auto &theStage = mWorkflow[stage];
		if (theStage.empty())
		{
			stepRet = conn->DefaultStep(cond, stage, request, response, buf, s);
			if (stepRet == StepResult::NTD_WAIT)
				return 0;
			if (stepRet == StepResult::NTD_ERROROCCURED)
				return -1;
			if (stepRet == StepResult::NTD_CONTINUE)
				return -1;
		}
		else
		{
			for (auto &entry : theStage)
			{
				stepRet = entry->DoStep(mConf.userData, conn, cond, stage, request, response, buf, s);
				if (stepRet == StepResult::NTD_WAIT)
					return 0;
				if (stepRet == StepResult::NTD_ERROROCCURED)
					return -1;
				if (stepRet == StepResult::NTD_COMPLETE)
					break;
				if (stepRet == StepResult::NTD_CONTINUE)
				{
				}
			}
			if (stepRet == NTD_CONTINUE)
			{
				stepRet = conn->DefaultStep(cond, stage, request, response, buf, s);
			}
		}

		if (stageOrig == stage)
		{
			if (stepRet != NTD_COMPLETE)
			{
				conn->SendSimpleTextRespond(404, "Not found", 9);
				continue;
			}
			break;
		}

		if (StepResult::NTD_COMPLETE == stepRet && stage == CS_MAX - 1)
		{
			stage = CS_WAIT_METHOD;
			if (!conn->mKeepAlive)
				return -1;
		}
		
		stageOrig = stage;
	}

	return 0;
}

void payHttpServer::OnAccept(evutil_socket_t sockfd, short event_type, void *arg)
{
	sockaddr_in si;
	memset(&si, 0, sizeof(si));
	socklen_t sockLen = sizeof(si);

	evutil_socket_t sock = accept(sockfd,
		reinterpret_cast<sockaddr*>(&si), &sockLen);
	if (sock < 0)
		return;

	payHttpServer *ws = reinterpret_cast<payHttpServer*>(arg);
	if (ws->mConf.flags & libhttpd::HSC_HTTPS)
	{
		SSLConnection::NewConnection(ws, sock);
	}
	else
	{
		Connection::NewConnection(ws, sock);
	}
}

void payHttpServer::OnMaintenanceTimer(int fd, short event, void *arg)
{
	auto ps = reinterpret_cast<payHttpServer*>(arg);
	ps->OnMaintenanceTimer(fd, event);
}

void payHttpServer::OnMaintenanceTimer(int fd, short event)
{
	mTimerCounter++;
	DWORD tick = notstd::GetCurrentTick();

	if (mTimerCounter >= 4)
	{
		for (auto iter = mHttpConnList.begin(); iter != mHttpConnList.end(); iter++)
		{
			(*iter)->OnMaintenance(tick);
		}
		mTimerCounter = 0;
	}
	else
	{
		for (auto iter = mHttpConnList.begin(); iter != mHttpConnList.end();)
		{
			if (Connection::DestroyConnection(*iter))
			{
				iter = mHttpConnList.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
	mMaintenanceVal.tv_sec = 1;
	mMaintenanceVal.tv_usec = 0;
	evtimer_add(mMaintenanceTimer, &mMaintenanceVal);
}

bool payHttpServer::StartPayHttpServer(libhttpd::HttpServConf *conf)
{
	if (conf)
		mConf = *conf;

	if (mConf.sizeOfMe != sizeof(*conf))
		return false;

	//WSADATA wsaData;
	//if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	//	return false;
	if (!notstd::NetEnviroment::Init())
		return false;
	mStartupOK = true;

	do {
		LoadExtModule();

		if (mConf.flags & libhttpd::HSC_HTTPS)
		{
			mSSLContext = new SSLContext;
			if (!mSSLContext->UseCertificateFile(mConf.certificateFilePathName))
			{
				LIBHTTP_LOGOUT("httpServerNew", "use certificate file fail\r\n");
				break;
			}

			if (!mSSLContext->UsePrivateKeyFile(mConf.privateKeyFilePathName, mConf.privateKeyPassword))
			{
				LIBHTTP_LOGOUT("httpServerNew", "use private key file fail\r\n");
				break;
			}

			if (mConf.flags & libhttpd::HSC_HTTPS_VERIFY_CLIENT)
			{
				mSSLContext->SetVerify(true);
				if (!SSL_CTX_load_verify_locations(*mSSLContext, mConf.clientCertFilePathName, nullptr))
				{
					LIBHTTP_LOGOUT("httpServerNew", "load client verify locations fail\r\n");
					break;
				}
				SSL_CTX_set_mode(*mSSLContext, SSL_MODE_AUTO_RETRY);
			}
		}

		assert(mListenSocket == INVALID_SOCKET);
		mListenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == mListenSocket) {
#ifdef PLATFORM_WINDOWS
			LIBHTTP_LOGOUT("httpServerNew", "socket fail {%lu}\r\n", ::GetLastError());
#endif
			break;
		}

		notstd::NetAddress na(mConf.ipv4Addr, mConf.port);

		//sockaddr_in si;
		//memset(&si, 0, sizeof(si));
		//si.sin_family = AF_INET;
		//auto ptonRet = ::inet_pton(AF_INET, mConf.ipv4Addr, &si.sin_addr.s_addr);
		//if (ptonRet == -1)
		//{
		//	LIBHTTP_LOGOUT("httpServerNew", "inet_pton(%s) fail {%lu}.\r\n", mConf.ipv4Addr, ::WSAGetLastError());
		//	break;
		//}
		//if (ptonRet == 0)
		//{
		//	LIBHTTP_LOGOUT("httpServerNew", "invalid IPv4 dotted-decimal string (%s)\r\n", mConf.ipv4Addr);
		//	break;
		//}
		//si.sin_port = ::htons(mConf.port);
		if (::bind(mListenSocket, reinterpret_cast<sockaddr*>(&na), sizeof(na)) != 0)
		{
#ifdef PLATFORM_WINDOWS
			LIBHTTP_LOGOUT("httpServerNew", "bind fail {%lu}\r\n", ::GetLastError());
#endif
			break;
		}
		if (::listen(mListenSocket, -1) != 0)
		{
#ifdef PLATFORM_WINDOWS
			LIBHTTP_LOGOUT("httpServerNew", "listen fail {%lu}\r\n", ::GetLastError());
#endif
			break;
		}

		assert(mEventBase == nullptr);
		assert(mListenEvent == nullptr);
		mEventBase = event_base_new();
		if (!mEventBase)
			break;

		evutil_make_socket_nonblocking(mListenSocket);
		mListenEvent = event_new(mEventBase, mListenSocket,
			EV_READ | EV_PERSIST, &payHttpServer::OnAccept, this);
		if (!mListenEvent)
			break;
		event_add(mListenEvent, nullptr);

		mMaintenanceTimer = evtimer_new(mEventBase, &payHttpServer::OnMaintenanceTimer, this);
		mMaintenanceVal.tv_sec = 1;
		mMaintenanceVal.tv_usec = 0;
		evtimer_add(mMaintenanceTimer, &mMaintenanceVal);

		//mCleanupThread = std::thread(std::bind(&payHttpServer::OnCleanup, this));
		mWorkingThread = std::thread([this]()
		{
			event_base_dispatch(mEventBase);
		});

		return true;
	} while (0);

	StopPayHttpServer();
	return false;
}

void payHttpServer::StopPayHttpServer()
{
	if (mEventBase)
	{
		event_base_loopbreak(mEventBase);
	}

	if (mMaintenanceTimer)
	{
		event_free(mMaintenanceTimer);
		mMaintenanceTimer = nullptr;
	}
	
	if (mListenEvent)
	{
		event_free(mListenEvent);
		mListenEvent = nullptr;
	}

	if (mEventBase)
	{
		event_base_free(mEventBase);
		mEventBase = nullptr;
	}

	// 清理链接
	for (auto iter = mHttpConnList.begin(); iter != mHttpConnList.end(); iter++)
	{
		(*iter)->DecRef();
	}
	mHttpConnList.clear();

	if (INVALID_SOCKET != mListenSocket)
	{
#ifdef PLATFORM_WINDOWS
		::closesocket(mListenSocket);
#else
		::close(mListenSocket);
#endif
		mListenSocket = INVALID_SOCKET;
	}

	if (mSSLContext)
	{
		delete mSSLContext;
		mSSLContext = nullptr;
	}

	UnloadExtModule();

	if (mStartupOK)
	{
		//::WSACleanup();
		notstd::NetEnviroment::Term();
		mStartupOK = false;
	}
}
