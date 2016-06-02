#include "StdAfx.h"
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
	return nullptr;
}

runtimeObjectBase* shortObject::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* shortObject::Mul(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* shortObject::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* shortObject::SetValue(runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* shortObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* shortObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* shortObject::getIndex(int i)
{
	return nullptr;
}

stringObject* shortObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%d", (int)mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
