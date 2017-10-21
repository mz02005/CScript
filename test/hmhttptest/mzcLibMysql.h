#pragma once
#include "CScriptEng/vm.h"
#include "CScriptEng/arrayType.h"

namespace mzcLib {
	class mysqlObject : public runtime::baseObjDefault
	{
	public:
		virtual runtime::runtimeObjectBase* GetMember(const char *memName);
	};
}
