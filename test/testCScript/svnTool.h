#pragma once
#include "CScriptEng/CScriptEng.h"

namespace tools {
	class svnTools : public runtime::baseObjDefault
	{
	public:
		virtual runtime::runtimeObjectBase* GetMember(const char *memName);
	};
}
