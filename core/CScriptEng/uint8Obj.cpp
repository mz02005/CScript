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
	return nullptr;
}

runtimeObjectBase* byteObject::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* byteObject::Mul(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* byteObject::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* byteObject::SetValue(runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* byteObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* byteObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* byteObject::getIndex(int i)
{
	return nullptr;
}

stringObject* byteObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%d", (int)mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
