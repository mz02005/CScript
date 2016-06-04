#include "StdAfx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

stringObject::stringObject()
	: mVal(new std::string)
{
}

stringObject::~stringObject()
{
	delete mVal;
}

uint32_t stringObject::GetObjectTypeId() const
{
	return DT_string;
}

runtimeObjectBase* stringObject::Add(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
		return nullptr;
	}

	runtimeObjectBase *r = nullptr;
	uint32_t typeId = obj->GetObjectTypeId();
	if (typeId == DT_string)
	{
		stringObject *val = new ObjectModule<stringObject>;
		*val->mVal = *mVal + *static_cast<const stringObject*>(obj)->mVal;
		r = val;
	}
	else
		return nullptr;

	return r;
}

runtimeObjectBase* stringObject::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* stringObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
		return nullptr;
	}

	runtimeObjectBase *r = nullptr;
	uint32_t typeId = obj->GetObjectTypeId();
	if (typeId == DT_int32)
	{
		// 通过乘法运算，能够迅速的构造重复的字符串
		stringObject *val = new ObjectModule<stringObject>;
		int count = static_cast<const intObject*>(obj)->mVal;
		for (int32_t i = 0; i < count; i ++)
			*val->mVal += *mVal;
		r = val;
	}
	else
		return nullptr;

	return r;
}

runtimeObjectBase* stringObject::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* stringObject::SetValue(runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
		return nullptr;
	}
	uint32_t typeId = obj->GetObjectTypeId();

	if (typeId == DT_string)
	{
		*mVal = *static_cast<const stringObject*>(obj)->mVal;
		return this;
	}
	
	return nullptr;
}

runtimeObjectBase* stringObject::GetMember(const char *memName)
{
	if (!strcmp(memName, "len"))
	{
		uintObject *obj = baseTypeObject::CreateBaseTypeObject<uintObject>(false);
		obj->mVal = mVal->size();
		return obj;
	}
	return __super::GetMember(memName);
}

runtimeObjectBase* stringObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* stringObject::getIndex(int i)
{
	std::string::size_type l = mVal->size();
	if (i < 0 || i >= l)
		return nullptr;
	intObject *r = new runtime::ObjectModule<intObject>;
	r->mVal = (int)mVal->operator[](i);
	return r;
}

stringObject* stringObject::toString()
{
	//AddRef();
	stringObject *r = new runtime::ObjectModule<stringObject>;
	*r->mVal = *mVal;
	return r;
}

bool stringObject::isGreaterThan(const runtimeObjectBase *obj)
{
	return *mVal > getObjectData<stringObject>(obj);
}

bool stringObject::isEqual(const runtimeObjectBase *obj)
{
	return *mVal == getObjectData<stringObject>(obj);
}

///////////////////////////////////////////////////////////////////////////////
