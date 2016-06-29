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
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	uintObject *val = new ObjectModule<uintObject>;
	val->mVal = mVal + getObjectDataOrig<uint32_t>(obj);
	return val;
}

runtimeObjectBase* uintObject::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("sub on const variable");
		return NULL;
	}
	
	if (!isNumberType(obj))
		return NULL;

	uintObject *val = new ObjectModule<uintObject>;
	val->mVal = mVal - getObjectDataOrig<uint32_t>(obj);
	return val;
}

runtimeObjectBase* uintObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return NULL;
	}
		
	if (!isNumberType(obj))
		return NULL;

	uintObject *val = new ObjectModule<uintObject>;
	val->mVal = mVal * getObjectDataOrig<uint32_t>(obj);
	return val;
}

runtimeObjectBase* uintObject::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return NULL;
	}
		
	if (!isNumberType(obj))
		return NULL;

	uintObject *val = new ObjectModule<uintObject>;
	int divisor = getObjectDataOrig<uint32_t>(obj);
	if (!divisor)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return NULL;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* uintObject::SetValue(runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* uintObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* uintObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* uintObject::getIndex(int i)
{
	return NULL;
}

stringObject* uintObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%u", mVal);
	return s;
}

bool uintObject::isGreaterThan(runtimeObjectBase *obj)
{
	if (isNumberType(obj))
	{
		return mVal > getObjectData<uintObject>(obj);
	}
	return false;
}

bool uintObject::isEqual(runtimeObjectBase *obj)
{
	return isNumberType(obj)
		&& (mVal == getObjectData<uintObject>(obj));
}

///////////////////////////////////////////////////////////////////////////////
