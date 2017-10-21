#include "stdafx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

shortObject::shortObject()
	: mVal(0)
{
}

uint32_t shortObject::GetObjectTypeId() const
{
	return DT_int16;
}

runtimeObjectBase* shortObject::Add(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* shortObject::Sub(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* shortObject::Mul(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* shortObject::Div(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* shortObject::SetValue(runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* shortObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* shortObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* shortObject::getIndex(int i)
{
	return NULL;
}

stringObject* shortObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	notstd::StringHelper::Format(*s->mVal, "%d", (int)mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
