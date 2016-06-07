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
	return NULL;
}

runtimeObjectBase* ushortObject::Sub(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* ushortObject::Mul(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* ushortObject::Div(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* ushortObject::SetValue(runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* ushortObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* ushortObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* ushortObject::getIndex(int i)
{
	return NULL;
}

stringObject* ushortObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%d", (int)mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
