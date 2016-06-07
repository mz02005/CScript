#include "StdAfx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

byteObject::byteObject()
	: mVal(0)
{
}

uint32_t byteObject::GetObjectTypeId() const
{
	return DT_uint8;
}

runtimeObjectBase* byteObject::Add(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* byteObject::Sub(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* byteObject::Mul(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* byteObject::Div(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* byteObject::SetValue(runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* byteObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* byteObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* byteObject::getIndex(int i)
{
	return NULL;
}

stringObject* byteObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%d", (int)mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
