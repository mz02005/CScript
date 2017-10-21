#include "stdafx.h"
#include "cRuntimeExt.h"

namespace tools {
	///////////////////////////////////////////////////////////////////////////

	class getenvObj : public runtime::baseTypeObject
	{
	public:
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			const char *envName;
			if (context->GetParamCount() != 1
				|| (envName = context->GetStringParam(0)) == nullptr)
				return runtime::NullTypeObject::CreateNullTypeObject();

			runtime::stringObject *r = new runtime::ObjectModule<runtime::stringObject>;
			char *v = ::getenv(envName);
			if (v)
				*r->mVal = v;

			return r;
		}
	};

	///////////////////////////////////////////////////////////////////////////

	runtime::runtimeObjectBase* CRuntimeExtObj::GetMember(const char *memName)
	{
		if (!strcmp("getenv", memName))
		{
			// 静态成员，不需要保存父对象
			return new runtime::ObjectModule<getenvObj>;
		}
		return runtime::baseObjDefault::GetMember(memName);
	}
}
