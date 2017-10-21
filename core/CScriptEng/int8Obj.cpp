#include "stdafx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

charObject::charObject()
	: mVal(0)
{
}

uint32_t charObject::GetObjectTypeId() const
{
	return DT_int8;
}

runtimeObjectBase* charObject::Add(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* charObject::Sub(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* charObject::Mul(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* charObject::Div(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* charObject::SetValue(runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* charObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* charObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* charObject::getIndex(int i)
{
	return NULL;
}

stringObject* charObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	notstd::StringHelper::Format(*s->mVal, "%c", mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
