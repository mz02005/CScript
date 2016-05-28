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
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
		return nullptr;
	}

	if (!isNumberType(obj))
		return nullptr;

	uintObject *val = new ObjectModule<uintObject>;
	val->mVal = mVal + getObjectDataOrig<uint32_t>(obj);
	return val;
}

runtimeObjectBase* uintObject::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("sub on const variable");
		return nullptr;
	}
	
	if (!isNumberType(obj))
		return nullptr;

	uintObject *val = new ObjectModule<uintObject>;
	val->mVal = mVal - getObjectDataOrig<uint32_t>(obj);
	return val;
}

runtimeObjectBase* uintObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return nullptr;
	}
		
	if (!isNumberType(obj))
		return nullptr;

	uintObject *val = new ObjectModule<uintObject>;
	val->mVal = mVal * getObjectDataOrig<uint32_t>(obj);
	return val;
}

runtimeObjectBase* uintObject::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return nullptr;
	}
		
	if (!isNumberType(obj))
		return nullptr;

	uintObject *val = new ObjectModule<uintObject>;
	int divisor = getObjectDataOrig<uint32_t>(obj);
	if (!divisor)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return nullptr;
	}
	val->mVal = mVal / divisor;
	return val;
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
