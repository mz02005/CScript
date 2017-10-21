#include "stdafx.h"
#include "hmhttptest.h"
#include "notstd/notstd.h"
#include "mzchtml.h"
#include <string>
#include <algorithm>

#ifdef PLATFORM_WINDOWS
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

#include <sys/stat.h>
#include <sys/types.h>

#ifdef min
#undef min
#endif

static const char pre[] = "/system/";
static const char shortPre[] = "/system";
static const size_t preLen = strlen(pre);

static const char getConf[] = "/getconfig";
static const size_t getConfLen = strlen(getConf);

#define RN "\r\n"

static const char pageSingleLine[] =
"<tr>" RN											//
"<th align=\"left\"><a href=\"%s\">%s</a></th>" RN	// url name
"<th align=\"left\">%s</th>" RN						// type(dir or file)
"<th align=\"left\">%s</th>" RN					// size
"<th align=\"right\">%s</th>" RN					// date
"</tr>" RN;											//

static const char pageBaseHead[] =
{
	"<html><head><title>%s</title><meta http-equiv=\"Content-Type\" content=\"text/html;charset=gb2312\"/></head><body><H1>%s</H1><hr>" RN
	"<table>" RN

	"<tr>" RN
	"<th align=\"left\">File Name</th>" RN
	"<th align=\"left\">File Type</th>" RN
	"<th align=\"left\">File Size</th>" RN
	"<th align=\"right\">File Date</th>" RN
	"<th align=\"right\"></th>" RN
	"</tr>" RN
	"<tr><th colspan=\"5\"><hr></th></tr>\r\n" RN
};

static const char pageBaseTail[] =
"</table></body></html>" RN;

struct FileInfo
{
	// 从文件的哪个位置
	uint64_t start;
	// 发送到文件的哪个位置
	uint64_t end;

	// 当前发送的偏移量（基于start）
	uint64_t sendOff;

	FILE *file;
};

struct DataInfo
{
	int isGetConf;
	int isRoot;
	int isFile;
	// 0代表system目录，其它值代表配置文件中的数据
	int pathBaseIndex;
	union {
		FileInfo fi;
	} infos;
};

static std::string GetFileSizeString(uint64_t fileSize)
{
	static const struct SizeEntry {
		uint64_t f;
		const char *unit;
	} se[] =
	{
		{ 1024 * 1024 * 1024, "G", },
		{ 1024 * 1024, "M", },
		{ 1024, "K", },
	};
	static const int entrySize = sizeof(se) / sizeof(se[0]);
	for (int i = 0; i < entrySize; i++)
	{
		uint64_t v = fileSize / se[i].f;
		if (v)
		{
			std::string r;
			return notstd::StringHelper::Format(r,
				"%.3f%s", ((double)fileSize) / se[i].f, se[i].unit);
		}
	}
	return std::to_string(fileSize);
}

inline std::string UrlToLocalPath(const std::string &url, int pathIndex)
{
	std::string path;

	if (!pathIndex)
	{
#ifdef PLATFORM_WINDOWS
		std::string ss = url.substr(preLen);
		if (!ss.size())
			return "";
		if (!isalpha(ss[0]))
			return "";
		if (ss.size() >= 2 && ss[1] != '/')
			return "";

		path += ss[0];
		path += ":\\";
		path += ss.size() >= 2 ? ss.substr(2) : ss.substr(1);
		notstd::StringHelper::Replace(path, "/", "\\");
#else
		std::string ss = url.substr(preLen - 1);
		if (ss.empty())
			return "/";
		return ss;
#endif
	}
	else
	{
		auto &pathInfo = Singleton<PathIndex>::GetInstance().GetPathTable()[pathIndex - 1];
		path = url.substr(pathInfo.first.size());
		path = std::get<0>(pathInfo.second) + path;
		notstd::StringHelper::Replace(path, "/", "\\");
	}

	return UrlHelper::UrlDecode(path);
}

static StepResult SendDataInner(void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	auto dBuf = resp->GetResponseBuffer();

	resp->SetStatusCode(200);
	resp->AppendHeader("server", "http server(by mz02005)");
	resp->AppendHeader("content-length", std::to_string(dBuf->GetTotalSize()).c_str());
	resp->AppendHeader("content-type", "text/html");
	
	stage = HttpStage::CS_SEND_Header;

	return NTD_COMPLETE;
}

static StepResult ListDirectory(const std::string &dirPath, void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	auto dBuf = resp->GetResponseBuffer();
	std::string toSend, temp;

	HttpRequestBaseInfo baseInfo;
	request->GetHttpRequestBaseInfo(&baseInfo);
	std::string baseUrl = baseInfo.url;
	if (baseUrl.back() != '/')
		baseUrl += "/";

	notstd::CFindIterator fi(dirPath.c_str());
	notstd::StringHelper::Format(toSend, pageBaseHead, baseInfo.url, baseInfo.url);
	for (notstd::CFindResult fr = fi.begin(); fr != fi.end(); fr = fi.next())
	{
		bool isDir = fr.IsDirectory();
		uint64_t size = 0;
		std::string datetime;
		if (!isDir)
		{
			//struct _stat64 statBuf;
			//struct tm theTm;
			//if (!_stat64(fr.GetPath().c_str(), &statBuf))
			//{
			//	size = statBuf.st_size;

			//	if (!_localtime64_s(&theTm, &statBuf.st_mtime))
			//	{
			//		notstd::StringHelper::Format(datetime, "%04d-%02d-%02d %02d:%02d:%02d",
			//			theTm.tm_year + 1900, theTm.tm_mon + 1, theTm.tm_mday,
			//			theTm.tm_hour, theTm.tm_min, theTm.tm_sec);
			//	}
			//}
			FileAttribute fa;
			if (File::GetFileAttribute(fr.GetPath(), &fa) >= 0)
			{
				size = fa.fileSize;
				datetime = Time(fa.lastWrittenTime).GetString();
			}
		}
		std::string nameAfterEncode = UrlHelper::UrlEncode(fr.GetName());
		toSend += notstd::StringHelper::Format(temp, pageSingleLine,
			(baseUrl + nameAfterEncode).c_str(), fr.GetName().c_str(), isDir ? "DIR" : "FILE",
			GetFileSizeString(size).c_str(), datetime.c_str());
	}
	toSend += pageBaseTail;

	auto singlePiece = dBuf->CreatePeice();
	singlePiece->AppendData(toSend.c_str(), toSend.size());
	dBuf->AppendPiece(singlePiece);
	return SendDataInner(userData, conn, cond, stage, request, resp, buf, s);
}

static int ParseRange(DataInfo *dataInfo, const std::string &range, const std::string &filePathName)
{
	static const char rangeHead[] = "bytes=";
	static const size_t rangeHeadLen = strlen(rangeHead);
	if (notstd::StringHelper::Left(range, rangeHeadLen) != rangeHead)
		return -1;

	auto f = range.find('-');
	if (f == range.npos)
		return -1;

	//struct _stat64 statBuf;
	//if (_stat64(filePathName.c_str(), &statBuf))
	//	return -1;
	FileAttribute fa;
	if (File::GetFileAttribute(filePathName, &fa) < 0)
		return -1;

	auto &fi = dataInfo->infos.fi;
	fi.start = strtoull(range.substr(rangeHeadLen, f - rangeHeadLen).c_str(), nullptr, 10);
	if (fi.start >= uint64_t(fa.fileSize))
		return -1;

	if (f == range.size() - 1)
	{
		fi.end = fa.fileSize ? fa.fileSize : fi.start;
	}
	else
	{
		fi.end = strtoull(range.substr(f + 1).c_str(), nullptr, 10);
		if (!fi.end)
			fi.end = fa.fileSize ? fa.fileSize : fi.start;
		else
		{
			if (fi.end > uint64_t(fa.fileSize))
				return -1;
		}
	}
	fi.sendOff = 0;

	return 0;
}

static StepResult ReturnFile(DataInfo *dataInfo, const std::string &filePathName, void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	std::string headBuf;
	headBuf.resize(512);
	size_t bufLen = 512;
	int r = request->GetHeader("range", &headBuf[0], &bufLen);

	//struct _stat64 statBuf;
	//if (_stat64(filePathName.c_str(), &statBuf))
	//	return conn->SendSimpleTextRespond(500, "Open file fail\r\n", -1);
	FileAttribute fa;
	if (File::GetFileAttribute(filePathName, &fa) < 0)
		return conn->SendSimpleTextRespond(500, "Open file fail\r\n", -1);

	resp->SetStatusCode(200);
	resp->AppendHeader("server", "http server(by mz02005)");

	// 这里解决下载中文文件的文件名为urlencode编码后的字符的问题
	auto fName = filePathName.rfind('\\');
	if (fName != filePathName.npos)
	{
		// 如果文件名都是可显示字符的，就不加上这个说明；否则就加上这个说明
		if (!std::all_of(&filePathName[fName], &filePathName[filePathName.size()], [](char c) {
			return isprint(c);
		}))
		{
			resp->AppendHeader("Content-Disposition",
				std::string("attachment; filename=\"").append(
				notstd::ICONVext::mbcsToUtf8(filePathName.substr(fName + 1)).append("\"")).c_str());
		}
	}

	dataInfo->isFile = 1;
	auto &fi = dataInfo->infos.fi;

	auto ff = filePathName.rfind('.');
	if (ff != filePathName.npos)
	{
		auto mimeType = Singleton<MimeManager>::GetInstance().GetMimeType(filePathName.substr(ff + 1));
		if (!mimeType.empty())
		{
			resp->AppendHeader("Content-Type", mimeType.c_str());
		}
	}

	if (r < 0)
	{
		fi.start = 0;
		fi.end = fa.fileSize ? fa.fileSize : fi.start;

		resp->AppendHeader("Content-Length", std::to_string(fa.fileSize).c_str());
	}
	else
	{
		headBuf.resize(bufLen);

		if (ParseRange(dataInfo, headBuf, filePathName) < 0)
			return NTD_ERROROCCURED;

		resp->AppendHeader("Content-Range",
			(
			std::string("bytes ") 
			+ std::to_string(fi.start) 
			+ "-" 
			+ std::to_string(fi.end) 
			+ "/" + std::to_string(fa.fileSize)
			).c_str());

		resp->AppendHeader("Content-Length", 
			std::to_string(
			fi.end == fi.start ? 
			0 : fi.end - fi.start + 1
			).c_str());
	}

	if (fi.start < fi.end)
	{
#ifdef PLATFORM_WINDOWS
		if (::fopen_s(&fi.file, filePathName.c_str(), "rb")
			|| !fi.file)
#else
		if ((fi.file = ::fopen(filePathName.c_str(), "rb")) == nullptr)
#endif
		{
			resp->ClearAllHeader();
			return conn->SendSimpleTextRespond(500, "Can't open file\r\n", -1);
		}
	}

	stage = HttpStage::CS_SEND_Header;
	return NTD_COMPLETE;
}

static std::string GetFileName(const std::string &pathName)
{
	auto f = pathName.rfind('\\');
	if (f != pathName.npos)
		return pathName.substr(f + 1);
	return pathName;
}

static std::string GetFileExt(const std::string &pathName)
{
	auto f = pathName.rfind('.');
	if (f != pathName.npos)
		return pathName.substr(f + 1);
	return pathName;
}

static StepResult ReleasePath(DataInfo *dataInfo, const std::string &localPath, 
	void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	FileAttribute fa;
	if (File::GetFileAttribute(localPath, &fa) < 0)
		return NTD_ERROROCCURED;
	if (fa.isDir())
	//if (::PathIsDirectoryA(localPath.c_str()))
	{
		return ListDirectory(localPath, userData, conn, cond, stage, request, resp, buf, s);
	}
#ifdef PLATFORM_WINDOWS
	if (notstd::StringHelper::Right(localPath, 8) == ".mzchtml")
	{
		return mzchtml::ReturnMZCHtml(localPath, conn, cond, stage, request, resp);
	}
#endif
	return ReturnFile(dataInfo, localPath, userData, conn, cond, stage, request, resp, buf, s);
}

#ifdef PLATFORM_WINDOWS
static StepResult ListRoot(void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
	IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
{
	auto dBuf = resp->GetResponseBuffer();

	HttpRequestBaseInfo baseInfo;
	request->GetHttpRequestBaseInfo(&baseInfo);

	std::string toSend;
	notstd::StringHelper::Format(toSend, pageBaseHead, baseInfo.url, baseInfo.url);
	for (char dr = 'a'; dr <= 'z'; dr++)
	{
		std::string str, temp;
		str += dr;
		str += ":\\";
		std::string drv;
		UINT r = ::GetDriveTypeA(str.c_str());
		if (r >= DRIVE_REMOVABLE
			&& r <= DRIVE_RAMDISK)
		{
			std::string theUrl = pre;
			theUrl += dr;
			theUrl += "/";
			std::string theName;
			theName += dr;
			theName += ":\\";
			toSend += notstd::StringHelper::Format(temp, pageSingleLine,
				theUrl.c_str(), theName.c_str(), "DIR", "0", "");
		}
	}
	toSend += pageBaseTail;

	auto singlePiece = dBuf->CreatePeice();
	singlePiece->AppendData(toSend.c_str(), toSend.size());
	dBuf->AppendPiece(singlePiece);

	return SendDataInner(userData, conn, cond, stage, request, resp, buf, s);
}
#endif

extern "C" {
	uint64_t GetModuleCapability()
	{
		return
				(1 << HttpStage::CS_POSITION_RESOURCE)
			|	(1 << HttpStage::CS_AUTHORITY)
			|	(1 << HttpStage::CS_GENERATE_RESPONSE_DATA)
			//|	(1 << HttpStage::CS_SEND_Header)
			|	(1 << HttpStage::CS_WAIT_HeaderSendOver)
			//|	(1 << HttpStage::CS_SEND_Body)
			|	(1 << HttpStage::CS_WAIT_BodySendOver)
			;
	}

	StepResult OnOPTIONS(const HttpRequestBaseInfo *baseInfo, DataInfo *dataInfo, void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
	{
		return conn->SendRespond(200, "text/plain", "", -1, [](IConnection *conn, IHttpRequest *request, IHttpResponse *response)->int
		{
			response->AppendHeader("Allow", "OPTIONS, GET, PUT, DELETE, MKCOL, PROPFIND");
			return 0;
		});
	}

	bool GetFileLength(const std::string &filePathName, int64_t &fileLen)
	{
		//struct _stat64 statBuf;
		//if (!_stat64(filePathName.c_str(), &statBuf))
		FileAttribute fa;
		if (File::GetFileAttribute(filePathName, &fa) >= 0)
		{
			//fileLen = statBuf.st_size;
			fileLen = fa.fileSize;
			return true;
		}
		return false;
	}

	StepResult OnPROPFIND(const HttpRequestBaseInfo *baseInfo, DataInfo *dataInfo, void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
	{
		int rCode = 200;
		std::string xmlData, temp;
		static const char retHeader[] =
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
			"<D:multistatus xmlns:D=\"DAV:\">\r\n";

		static const char retTail[] =
			"</D:multistatus>\r\n";

		static const char retBody[] =
			"<D:response>\r\n"
			"<D:href>%s</D:href>\r\n"
			"<D:propstat>\r\n"
			"<D:prop>\r\n"
			"<D:displayname>%s</D:displayname>\r\n"
			"<D:creationdate>%s</D:creationdate>\r\n"
			"<D:resourcetype>%s</D:resourcetype>\r\n"
			"<D:getcontentlength>%u</D:getcontentlength>\r\n"
			"</D:prop>\r\n"
			"</D:propstat>\r\n"
			"</D:response>\r\n";

		auto &ht = Singleton<httptest>::GetInstance();

		//DWORD fa = GetFileAttributesA(baseInfo->resLocalPosition);
		std::string baseUrl = baseInfo->url;
		if (baseUrl.back() != '/')
			baseUrl += '/';
		do {
			FileAttribute fa;
			if (File::GetFileAttribute(baseInfo->resLocalPosition, &fa) < 0)
			//if (INVALID_FILE_ATTRIBUTES == fa)
			{
				rCode = 404;
				break;
			}
			xmlData = retHeader;
			//if (fa & FILE_ATTRIBUTE_DIRECTORY)
			if (fa.isDir())
			{
				notstd::CFindIterator fi(baseInfo->resLocalPosition);
				for (notstd::CFindResult fr = fi.begin(); fr != fi.end(); fr = fi.next())
				{
					bool isDir = fr.IsDirectory();

					if (!isDir && dataInfo->pathBaseIndex > 0)
					{
						auto &pathInfo = Singleton<PathIndex>::GetInstance().GetPathTable()[dataInfo->pathBaseIndex - 1];
						auto &whiteList = std::get<3>(pathInfo.second);
						auto &blackList = std::get<4>(pathInfo.second);
						if (whiteList->size())
						{
							if (std::find(whiteList->begin(), whiteList->end(), fr.GetSuffix()) == whiteList->end())
								continue;
						}
						if (blackList->size())
						{
							if (std::find(blackList->begin(), blackList->end(), fr.GetSuffix()) != blackList->end())
								continue;
						}
					}

					int64_t fileLen = 0;
					GetFileLength(fr.GetPath(), fileLen);
					xmlData += notstd::StringHelper::Format(temp, retBody, 
						(baseUrl + fr.GetName()).c_str(), fr.GetName().c_str(),
						"Not defined", isDir ? "<D:collection/>" : "", (uint32_t)fileLen);
				}
			}
			else
			{
				int64_t fileLen = 0;
				std::string name = baseInfo->resLocalPosition;
				name = name.substr(name.rfind(Path::GetSplashCharacter()) + 1);

				GetFileLength(baseInfo->resLocalPosition, fileLen);

				xmlData += notstd::StringHelper::Format(temp, retBody,
					baseInfo->url, name.c_str(),
					"Not defined", "", (uint32_t)fileLen);
			}
			xmlData += retTail;
			xmlData = notstd::ICONVext::mbcsToUtf8(xmlData);
			return conn->SendRespond(rCode, "text/xml; charset=\"utf-8\"", xmlData.c_str(), xmlData.size());
		} while (0);

		return conn->SendSimpleTextRespond(rCode, "", -1);
	}

	StepResult OnDoReleaseData(const HttpRequestBaseInfo *baseInfo, DataInfo *dataInfo, void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
	{
		switch (baseInfo->verb)
		{
		case HttpConst::V_GET:
			return ReleasePath(dataInfo, baseInfo->resLocalPosition, userData, conn, cond, stage, request, resp, buf, s);

		case HttpConst::V_DELETE:
			do {
				if (std::remove(baseInfo->resLocalPosition) < 0)
					return conn->SendSimpleTextRespond(423, "Locked\r\n", -1);
				return conn->SendSimpleTextRespond(204, "No content\r\n", -1);
			} while (0);
			break;

		case HttpConst::V_MKCOL:
			do {
				if (!Path::CreateDirectoryRecursion(baseInfo->resLocalPosition))
					return conn->SendSimpleTextRespond(409, "", -1);
				return conn->SendSimpleTextRespond(201, "", -1);
			} while (0);
			break;

		case HttpConst::V_COPY:
			return conn->SendSimpleTextRespond(404, "Not found", -1);

		case HttpConst::V_PUT:
			do {
				std::string data;
				auto rb = request->GetReceiveBuffer();
				if (rb->GetFirstPiece() >= 0)
				{
					IBufferPiece *piece = nullptr;
					while (rb->GetNextPiece(piece) >= 0)
					{
						void *buf;
						size_t s;
						if (piece->GetBuffer(buf, s) >= 0)
							data.append(reinterpret_cast<const char*>(buf), s);
					}
				}
				Handle<STDCFILEHandle> file = ::fopen(baseInfo->resLocalPosition, "w+b");
				if (!file)
					return conn->SendSimpleTextRespond(404, "");
				::fwrite(data.c_str(), 1, data.size(), file);
				return conn->SendSimpleTextRespond(201, "");
			} while (0);
			break;

		case HttpConst::V_MOVE:
			return conn->SendSimpleTextRespond(404, "Not found", -1);
			
		case HttpConst::V_PROPFIND:
			return OnPROPFIND(baseInfo, dataInfo, userData, conn, cond, stage, request, resp, buf, s);

		case HttpConst::V_OPTIONS:
			return OnOPTIONS(baseInfo, dataInfo, userData, conn, cond, stage, request, resp, buf, s);
		}
		return conn->SendSimpleTextRespond(500, "Not supportted\r\n", -1);
	}
	
	StepResult DoStep(void *userData, IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp, const void *buf, size_t &s)
	{
		static int useMyAddr = 1;
		DataInfo *data = nullptr;
		if (StepFireCond::SFC_WRITE != cond)
		{
			HttpRequestBaseInfo baseInfo;
			if (request->GetHttpRequestBaseInfo(&baseInfo) < 0)
			{
				std::string err;
				notstd::StringHelper::Format(err, "Inner error at %s@%d\r\n", __FILE__, __LINE__);
				return conn->SendSimpleTextRespond(500, err.c_str(), err.size());
			}

			std::string theUrl = baseInfo.url;

			if (HttpStage::CS_POSITION_RESOURCE == stage)
			{
				if (notstd::StringHelper::Left(theUrl, getConfLen) == getConf)
				{
					if (theUrl.size() == getConfLen + 1
						&& theUrl[getConfLen] != '/')
						return NTD_CONTINUE;

					if (theUrl.size() > getConfLen + 1)
						return NTD_CONTINUE;

					data = reinterpret_cast<DataInfo*>(conn->CreateData(&useMyAddr, sizeof(DataInfo)));
					data->isGetConf = 1;
					conn->GoNextStage(stage);
					return NTD_COMPLETE;
				}
				else
				{
					do {
						if (notstd::StringHelper::Left(theUrl, preLen - 1) != shortPre)
							break;
						if (theUrl.size() >= preLen && theUrl[preLen - 1] != '/')
							break;

						data = reinterpret_cast<DataInfo*>(conn->CreateData(&useMyAddr, sizeof(DataInfo)));
						memset(data, 0, sizeof(DataInfo));

						auto &fi = data->infos.fi;
						fi.start = 0;
						fi.end = -1;
						fi.sendOff = 0;
#ifdef PLATFORM_WINDOWS
						if (theUrl.size() <= preLen)
						{
							data->isRoot = 1;
							conn->GoNextStage(stage);
						}
						else
						{
#endif
							data->isRoot = 0;
							conn->GoNextStage(stage);

							std::string localPath = UrlToLocalPath(theUrl, data->pathBaseIndex);
							if (localPath.empty())
								return conn->SendSimpleTextRespond(404, "File not found", -1);

							request->SetResourceLocalPosition(localPath.c_str(), localPath.size());
#ifdef PLATFORM_WINDOWS
						}
#endif
						return NTD_COMPLETE;
					} while (0);

					int xx = Singleton<PathIndex>::GetInstance().GetPathIndex(theUrl);
					if (xx > 0)
					{
						data = reinterpret_cast<DataInfo*>(conn->CreateData(&useMyAddr, sizeof(DataInfo)));
						memset(data, 0, sizeof(DataInfo));
						data->pathBaseIndex = xx;

						std::string localPath = UrlToLocalPath(theUrl, data->pathBaseIndex);
						if (localPath.empty())
							return conn->SendSimpleTextRespond(404, "File not found", -1);

						request->SetResourceLocalPosition(localPath.c_str(), localPath.size());

						conn->GoNextStage(stage);
						return NTD_COMPLETE;
					}

					return NTD_CONTINUE;
				}
			}
			else if (CS_AUTHORITY == stage)
			{
				data = reinterpret_cast<DataInfo*>(conn->FindData(&useMyAddr));
				if (!data)
					return NTD_CONTINUE;

				auto &theHttpTest = Singleton<httptest>::GetInstance();

				if (!data->pathBaseIndex)
				{
					auto &basicAuth = theHttpTest.mFileSystem.mBasicAuth;
					if (basicAuth.mUsername.mVal.size() || basicAuth.mPassword.mVal.size())
					{
						if (request->DoBasicAuthenticate(basicAuth.mUsername.mVal.c_str(),
							basicAuth.mPassword.mVal.c_str()) < 0)
							return conn->Send401Respond("need basic auth");
					}
				}
				else
				{
					const auto &item = Singleton<PathIndex>::GetInstance().GetPathTable()[data->pathBaseIndex - 1].second;
					auto &userName = std::get<1>(item);
					auto &password = std::get<2>(item);
					if (userName.size() && password.size())
					{
						if (request->DoBasicAuthenticate(userName.c_str(), password.c_str()) < 0)
							return conn->Send401Respond("need basic auth");
					}
				}

				conn->GoNextStage(stage);
				return NTD_COMPLETE;
			}
			else if (HttpStage::CS_GENERATE_RESPONSE_DATA == stage)
			{
				if (!(data = (DataInfo*)conn->FindData(&useMyAddr)))
					return NTD_CONTINUE;
				
				if (data->isGetConf)
				{
					std::string path = notstd::AppHelper::GetAppPath();
					path.append(".xml");

					notstd::XmlElemSerializer serialer(path);
					auto &theHttpTest = Singleton<httptest>::GetInstance();
					serialer.Store(&theHttpTest);

					return conn->SendSimpleTextRespond(200, "configuration file is written\r\n", -1);
				}
				else {
#ifdef PLATFORM_WINDOWS
					if (data->isRoot)
					{
						if (baseInfo.verb != HttpConst::V_GET)
							return conn->SendSimpleTextRespond(423, "Locked\r\n", -1);
						return ListRoot(userData, conn, cond, stage, request, resp, buf, s);
					}
					else
					{
#endif
						return OnDoReleaseData(&baseInfo, data, userData, conn, cond, stage, request, resp, buf, s);
#ifdef PLATFORM_WINDOWS
					}
#endif
				}
			}
		}
		else
		{
			if (!(data = (DataInfo*)conn->FindData(&useMyAddr)))
				return NTD_CONTINUE;

			if (HttpStage::CS_WAIT_HeaderSendOver == stage)
			{
				if (data->isRoot || !data->isFile)
					return NTD_CONTINUE;

				if (data->isGetConf)
					return NTD_CONTINUE;

				// 注意，跳过了编码阶段，直接发送body缓冲
				stage = CS_SEND_Body;
				cond = StepFireCond::SFC_OTHER;

				auto &fi = data->infos.fi;
				if (!fi.file)
					return NTD_COMPLETE;

				// 发送一部分数据
				auto respBuf = resp->GetResponseBuffer();
				auto singlePiece = respBuf->CreatePeice();
				
				char readBuf[16384];
				size_t toRead = std::min(sizeof(readBuf), size_t(Singleton<httptest>::GetInstance().mFileSystem.mNetwork.mSendBufSize.mVal));
				toRead = std::min(size_t(toRead), size_t(fi.end - fi.sendOff));
				auto readed = ::fread(readBuf, 1, 
					toRead, fi.file);
				singlePiece->AppendData(readBuf, readed);
				respBuf->AppendPiece(singlePiece);

				stage = HttpStage::CS_SEND_Body;
				cond = StepFireCond::SFC_OTHER;
				data->infos.fi.sendOff += readed;

				return NTD_COMPLETE;
			}
			else if (HttpStage::CS_WAIT_BodySendOver == stage)
			{
				if (data->isGetConf)
					return NTD_CONTINUE;

				if (!data->isFile)
					return NTD_CONTINUE;

				if (!data->infos.fi.file)
					return conn->SendSimpleTextRespond(404, "File not found\r\n", -1);

				auto &fi = data->infos.fi;
				if (fi.start + fi.sendOff >= fi.end)
				{
					::fclose(data->infos.fi.file);
					stage = CS_WAIT_FOR_NEWROUND;
					return NTD_COMPLETE;
				}

				// 发送一部分数据
				auto respBuf = resp->GetResponseBuffer();
				respBuf->RemoveAll();
				auto singlePiece = respBuf->CreatePeice();

				char readBuf[16384];
				size_t toRead = std::min(sizeof(readBuf), size_t(Singleton<httptest>::GetInstance().mFileSystem.mNetwork.mSendBufSize.mVal));
				toRead = std::min(size_t(toRead), size_t(fi.end - fi.sendOff));
				auto readed = ::fread(readBuf, 1, toRead, fi.file);
				singlePiece->AppendData(readBuf, readed);
				respBuf->AppendPiece(singlePiece);

				stage = HttpStage::CS_SEND_Body;
				cond = StepFireCond::SFC_OTHER;
				data->infos.fi.sendOff += readed;

				return NTD_COMPLETE;
			}
			else
				return NTD_ERROROCCURED;
		}

		return NTD_COMPLETE;
	}

	int InitializeModule()
	{
		notstd::objBase::RegistAllClass();
		Singleton<MimeManager>::GetInstance().LoadMimeConf();
		do {
			auto &theHttpTest = Singleton<httptest>::GetInstance();

			std::string path = notstd::AppHelper::GetAppPath();
			path.append(".xml");

			notstd::XmlElemSerializer serialer(path);
			serialer.Load(&theHttpTest);

			Singleton<PathIndex>::GetInstance().LoadTableFromHttpTest(theHttpTest);
		} while (0);
		return 0;
	}

	int UninitializeModule()
	{
		return 0;
	}
}
