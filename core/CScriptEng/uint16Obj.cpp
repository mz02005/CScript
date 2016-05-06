#include "StdAfx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

ushortObject::ushortObject()
	: mVal(0)
{
}

uint32_t ushortObject::GetObjectTypeId() const
{
	return DT_uint16;
}

runtimeObjectBase* ushortObject::Add(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* ushortObject::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* ushortObject::Mul(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* ushortObject::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* ushortObject::SetValue(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* ushortObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* ushortObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* ushortObject::getIndex(int i)
{
	return nullptr;
}

stringObject* ushortObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%d", (int)mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
