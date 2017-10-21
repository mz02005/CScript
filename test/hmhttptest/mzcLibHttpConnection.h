#pragma once
#include "CScriptEng/vm.h"
#include "CScriptEng/arrayType.h"
#include "core/libhttp/httpinterface.h"

namespace mzcLib {
	class httpRequestObj;

	class HttpConnectionObj : public runtime::baseObjDefault
	{
		friend class httpRequestObj;

	private:
		runtime::arrayObject *mUserData;
		IHttpRequest *mHttpRequest;
		
	public:
		HttpConnectionObj();
		virtual ~HttpConnectionObj();
		virtual runtime::runtimeObjectBase* GetMember(const char *memName);
		static HttpConnectionObj* CreateHttpConnection(IHttpRequest *request);
	};
}
