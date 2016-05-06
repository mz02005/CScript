#include "StdAfx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

uintObject::uintObject()
	: mVal(0)
{
}

uint32_t uintObject::GetObjectTypeId() const
{
	return DT_uint32;
}

runtimeObjectBase* uintObject::Add(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* uintObject::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* uintObject::Mul(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* uintObject::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* uintObject::SetValue(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* uintObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* uintObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* uintObject::getIndex(int i)
{
	return nullptr;
}

stringObject* uintObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%u", mVal);
	return s;
}

bool uintObject::isGreaterThan(const runtimeObjectBase *obj)
{
	if (isNumberType(obj))
	{
		return mVal > getObjectData<uintObject>(obj);
	}
	return false;
}

bool uintObject::isEqual(const runtimeObjectBase *obj)
{
	return isNumberType(obj)
		&& (mVal == getObjectData<uintObject>(obj));
}

///////////////////////////////////////////////////////////////////////////////
