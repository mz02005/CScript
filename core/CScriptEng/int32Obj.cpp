#include "StdAfx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

intObject::intObject()
	: mVal(0)
{
}

uint32_t intObject::GetObjectTypeId() const
{
	return DT_int32;
}

runtimeObjectBase* intObject::Add(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
		return nullptr;
	}

	if (!isNumberType(obj))
		return nullptr;

	intObject *val = new ObjectModule<intObject>;
	val->mVal = mVal + getObjectDataOrig<int>(obj);
	return val;
}

runtimeObjectBase* intObject::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("sub on const variable");
		return nullptr;
	}
	
	if (!isNumberType(obj))
		return nullptr;

	intObject *val = new ObjectModule<intObject>;
	val->mVal = mVal - getObjectDataOrig<int>(obj);
	return val;
}

runtimeObjectBase* intObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return nullptr;
	}
		
	if (!isNumberType(obj))
		return nullptr;

	intObject *val = new ObjectModule<intObject>;
	val->mVal = mVal * getObjectDataOrig<int>(obj);
	return val;
}

runtimeObjectBase* intObject::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return nullptr;
	}
		
	if (!isNumberType(obj))
		return nullptr;

	intObject *val = new ObjectModule<intObject>;
	int divisor = getObjectDataOrig<int>(obj);
	if (!divisor)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return nullptr;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* intObject::SetValue(const runtimeObjectBase *obj)
{
	if (!isNumberType(obj))
		return nullptr;
	mVal = getObjectDataOrig<int>(obj);
	return this;
}

runtimeObjectBase* intObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* intObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* intObject::getIndex(int i)
{
	return nullptr;
}

stringObject* intObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%d", mVal);
	return s;
}

bool intObject::isGreaterThan(const runtimeObjectBase *obj)
{
	if (isNumberType(obj))
	{
		return mVal > getObjectData<intObject>(obj);
	}
	return false;
}

bool intObject::isEqual(const runtimeObjectBase *obj)
{
	return isNumberType(obj)
		&& (mVal == getObjectData<intObject>(obj));
}

intObject* intObject::CreateIntObject(int v)
{
	intObject *r = new runtime::ObjectModule<intObject>;
	r->mVal = v;
	return r;
}

///////////////////////////////////////////////////////////////////////////////