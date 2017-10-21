#include "stdafx.h"
#include "mzchtml.h"
#include "notstd/notstd.h"
#include "CScriptEng/vm.h"
#include "CScriptEng/arrayType.h"
#include "mzcLibMysql.h"
#include "mzcLibHttpConnection.h"

//
// <% 
//  RenderDefaultHtmlHeader('test'); 
//  RenderDefaultHtmlBody();
//  RenderDefaultHtmlTail();
// %>
//

// <!Doctype html>
// <html xmlns=\"http://www.w3.org/1999/xhtml\"><head><title>test<title></head>
// <body></body>
// </html>

namespace mzchtml {
	StepResult ReturnMZCHtml(const std::string &pathName,
		IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp)
	{
		Handle<STDCFILEHandle> file = ::fopen(pathName.c_str(), "rb");
		if (!file)
			return conn->SendSimpleTextRespond(404, "source file not found");

		fseek(file, 0, SEEK_END);
		auto ss = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		std::string fileDatas;
		fileDatas.resize(ss);
		size_t realGet = fread(&fileDatas[0], 1, ss, file);
		fclose(file);
		fileDatas.resize(realGet);

		// 把theConn拉出来，作为整个处理过程中的全局对象，可以多次给代码使用
		auto *theConn = mzcLib::HttpConnectionObj::CreateHttpConnection(request);
		theConn->AddRef();

		int httpCode = 200;
		std::string prompt;

		std::string::size_type f, g, base = 0;
		std::string finalHtml, finalMimeType = "text/html";
		for (; (f = fileDatas.find("<%", base)) != fileDatas.npos; base = g + 2)
		{
			finalHtml += fileDatas.substr(base, f - base);

			g = fileDatas.find("%>", base + 2);
			if (g == fileDatas.npos)
			{
				httpCode = 500;
				prompt = "Invalid mzchtml format";
				break;
			}

			scriptAPI::SimpleCScriptEng::Init();
			scriptAPI::StringStream fs(fileDatas.c_str() + f + 2, g - f - 2);
			scriptAPI::ScriptCompiler compiler;

			runtime::stringObject *htmlResultVal = new runtime::ObjectModule<runtime::stringObject>;
			runtime::stringObject *mimeType = new runtime::ObjectModule<runtime::stringObject>;
			//*htmlResultVal->mVal = "";
			*mimeType->mVal = finalMimeType;

			scriptAPI::ScriptLibReg les[] =
			{
				{ "htmlResult", htmlResultVal, },
				{ "mimeType", mimeType, },
				{ "mysql", new runtime::ObjectModule<mzcLib::mysqlObject>, },
				{ "httpConnection", theConn, },
			};
			compiler.GetLibRegister().RegistLib("mzchtmlResult", les, sizeof(les) / sizeof(les[0]));
			HANDLE cr = compiler.Compile(&fs, true);

			bool fail = true;
			scriptAPI::ScriptRuntimeContext *runtimeContext
				= scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext(1024, 512);
			runtimeContext->PushRuntimeObjectInLibs(&compiler.GetLibRegister());
			do {
				if (!cr) {
					prompt = "Compile mzchtml fail";
					break;
				}

				int exitCode = 0;
				int er = runtimeContext->Execute(cr, true, &exitCode);

				if (er == runtime::EC_Normal)
				{
				}
				else
				{
					prompt = "Execute mzchtml fail";
					break;
				}

				fail = false;
			} while (0);

			finalHtml += *htmlResultVal->mVal;
			finalMimeType = *mimeType->mVal;
			if (cr)
				scriptAPI::ScriptCompiler::ReleaseCompileResult(cr);
			scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(runtimeContext);
			scriptAPI::SimpleCScriptEng::Term();

			if (fail) {
				httpCode = 404;
				break;
			}
		}

		theConn->Release();

		if (httpCode != 200)
			return conn->SendSimpleTextRespond(httpCode, prompt.c_str(), prompt.size());

		if (f == fileDatas.npos)
			finalHtml += fileDatas.substr(base);

		return conn->SendRespond(200, finalMimeType.c_str(), finalHtml.c_str(), finalHtml.size());
	}
}
