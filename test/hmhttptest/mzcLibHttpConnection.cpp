#include "stdafx.h"
#include "mzcLibHttpConnection.h"

namespace mzcLib {
	class httpRequestObj;
	class HttpBasicInfoObj;

	class GetHeaderInHttpRequestObj : public runtime::baseTypeObject
	{
		friend class httpRequestObj;

	private:
		httpRequestObj *mRequestObj;

	public:
		GetHeaderInHttpRequestObj();
		virtual ~GetHeaderInHttpRequestObj();
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class GetParamInHttpRequestObj : public runtime::baseTypeObject
	{
		friend class httpRequestObj;

	private:
		httpRequestObj *mRequestObj;

	public:
		GetParamInHttpRequestObj();
		virtual ~GetParamInHttpRequestObj();
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class HttpBasicInfoObj : public runtime::baseTypeObject
	{
		friend class httpRequestObj;

	private:
		HttpRequestBaseInfo mBasicInfo;

	public:
		HttpBasicInfoObj()
		{
			memset(&mBasicInfo, 0, sizeof(mBasicInfo));
		}

		virtual runtime::runtimeObjectBase* GetMember(const char *memName) override
		{
			if (!strcmp("Verb", memName))
			{
				static const char *verbDesc[] =
				{
					"V_GET",
					"V_POST",
					"V_PUT",
					"V_OPTIONS",
					"V_HEAD",
					"V_DELETE",
					"V_TRACK",
					"V_COPY",
					"V_PROPFIND",
					"V_PROPPATCH",
					"V_MKCOL",
					"V_MOVE",
					"V_SEARCH",
				};

				auto *r = new runtime::ObjectModule<runtime::stringObject>;
				*r->mVal = mBasicInfo.verb >= sizeof(verbDesc) / sizeof(verbDesc[0]) ? "Unknown Verb" : verbDesc[mBasicInfo.verb];
				return r;
			}
			else if (!strcmp("Url", memName))
			{
				auto *r = new runtime::ObjectModule<runtime::stringObject>;
				*r->mVal = mBasicInfo.url;
				return r;
			}
			else if (!strcmp("Version", memName))
			{
				auto *r = new runtime::ObjectModule<runtime::stringObject>;
				*r->mVal = mBasicInfo.version;
				return r;
			}
			else if (!strcmp("LocalPosition", memName))
			{
				auto *r = new runtime::ObjectModule<runtime::stringObject>;
				if (mBasicInfo.posStringLen)
					r->mVal->append(mBasicInfo.resLocalPosition, mBasicInfo.posStringLen);
				return r;
			}

			return runtime::baseObjDefault::GetMember(memName);
		}
	};

	class httpRequestObj : public runtime::baseTypeObject
	{
		friend class HttpConnectionObj;
		friend class GetParamInHttpRequestObj;
		friend class GetHeaderInHttpRequestObj;

	private:
		HttpConnectionObj *mConnObj;

	private:
		IHttpRequest* GetHttpRequest()
		{
			return mConnObj->mHttpRequest;
		}

	public:
		httpRequestObj()
			: mConnObj(nullptr)
		{
		}

		virtual ~httpRequestObj()
		{
			if (mConnObj)
			{
				mConnObj->Release();
			}
		}

		virtual runtime::runtimeObjectBase* GetMember(const char *memName) override
		{
			if (!strcmp("BasicInfo", memName))
			{
				auto *r = new runtime::ObjectModule<HttpBasicInfoObj>;
				mConnObj->mHttpRequest->GetHttpRequestBaseInfo(&r->mBasicInfo);
				return r;
			}
			else if (!strcmp("GetParam", memName))
			{
				// 得到指定的Param
				auto *r = new runtime::ObjectModule<GetParamInHttpRequestObj>;
				r->mRequestObj = this;
				AddRef();
				return r;
			}
			else if (!strcmp("GetHeader", memName))
			{
				// 得到指定的Header
				auto *r = new runtime::ObjectModule<GetHeaderInHttpRequestObj>;
				r->mRequestObj = this;
				AddRef();
				return r;
			}
			else if (!strcmp("Params", memName))
			{
				// 得到所有的Param
				// 返回一个数组，数组的每个元素又是一个数组，其中含有两个元素，一个是名称，一个是值
				auto *request = mConnObj->mHttpRequest;
				if (request->GetFirstParam() < 0)
					return runtime::NullTypeObject::CreateNullTypeObject();

				auto *r = new runtime::ObjectModule<runtime::arrayObject>;

				char bufName[1024], bufVal[1024];
				size_t nameSize = sizeof(bufName), valSize = sizeof(bufVal);

				while (request->GetNextParam(bufName, &nameSize, bufVal, &valSize) >= 0)
				{
					auto *single = new runtime::ObjectModule<runtime::arrayObject>;
					auto *left = new runtime::ObjectModule<runtime::stringObject>;
					auto *right = new runtime::ObjectModule<runtime::stringObject>;
					left->mVal->append(bufName, nameSize);
					right->mVal->append(bufVal, valSize);
					single->AddSub(left);
					single->AddSub(right);
					r->AddSub(single);

					nameSize = sizeof(bufName);
					valSize = sizeof(bufVal);
				}
				return r;
			}

			return runtime::baseObjDefault::GetMember(memName);
		}
	};

	HttpConnectionObj::HttpConnectionObj()
		: mHttpRequest(nullptr)
		, mUserData(nullptr)
	{
	}

	HttpConnectionObj::~HttpConnectionObj()
	{
		if (mUserData)
			mUserData->Release();
	}

	runtime::runtimeObjectBase* HttpConnectionObj::GetMember(const char *memName)
	{
		if (!strcmp("HttpRequest", memName))
		{
			auto *r = new runtime::ObjectModule<httpRequestObj>;
			r->mConnObj = this;
			AddRef();
			return r;
		}
		else if (!strcmp("ConnData", memName))
		{
			if (!mUserData)
			{
				mUserData = new runtime::ObjectModule<runtime::arrayObject>;
				mUserData->AddRef();
			}
			return mUserData;
		}
		return runtime::baseObjDefault::GetMember(memName);
	}

	HttpConnectionObj* HttpConnectionObj::CreateHttpConnection(IHttpRequest *request)
	{
		auto *r = new runtime::ObjectModule<HttpConnectionObj>;
		r->mHttpRequest = request;
		return r;
	}

	////////////////////////////////////////////////////////////////////////////

	GetParamInHttpRequestObj::GetParamInHttpRequestObj()
		: mRequestObj(nullptr)
	{
	}

	GetParamInHttpRequestObj::~GetParamInHttpRequestObj()
	{
		if (mRequestObj)
			mRequestObj->Release();
	}

	runtime::runtimeObjectBase* GetParamInHttpRequestObj::doCall(runtime::doCallContext *context)
	{
		auto *r = new runtime::ObjectModule<runtime::stringObject>;
		if (context->GetParamCount() < 1)
			return r;

		const char *paramName = context->GetStringParam(0);
		if (!paramName)
			return r;

		char buf[1024] = { 0 };
		size_t bufSize = sizeof(buf);
		if (mRequestObj->GetHttpRequest()->GetParam(paramName, buf, &bufSize) >= 0)
		{
			*r->mVal = buf;
		}
		return r;
	}
	
	////////////////////////////////////////////////////////////////////////////

	GetHeaderInHttpRequestObj::GetHeaderInHttpRequestObj()
		: mRequestObj(nullptr)
	{
	}

	GetHeaderInHttpRequestObj::~GetHeaderInHttpRequestObj()
	{
		if (mRequestObj)
			mRequestObj->Release();
	}

	runtime::runtimeObjectBase* GetHeaderInHttpRequestObj::doCall(runtime::doCallContext *context)
	{
		do {
			if (context->GetParamCount() < 1)
				break;

			const char *headerName = context->GetStringParam(0);
			if (!headerName)
				break;

			char buf[1024] = { 0 };
			size_t bufSize = sizeof(buf);
			if (mRequestObj->GetHttpRequest()->GetHeader(headerName, buf, &bufSize) < 0)
				break;

			auto *r = new runtime::ObjectModule<runtime::stringObject>;
			*r->mVal = buf;

			return r;
		} while (0);

		return runtime::NullTypeObject::CreateNullTypeObject();
	}
}
