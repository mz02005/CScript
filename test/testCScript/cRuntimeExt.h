#pragma once
#include "CScriptEng/vm.h"
#include "CScriptEng/arrayType.h"

namespace tools {
	class CRuntimeExtObj : public runtime::baseObjDefault
	{
	public:
		virtual runtime::runtimeObjectBase* GetMember(const char *memName);
	};
}
