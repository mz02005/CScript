#include "StdAfx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

floatObject::floatObject()
	: mVal(0.f)
{
}

uint32_t floatObject::GetObjectTypeId() const
{
	return DT_float;
}

runtimeObjectBase* floatObject::Add(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Add on const variable");
		return nullptr;
	}
			
	if (!isNumberType(obj))
		return nullptr;

	floatObject *val = new ObjectModule<floatObject>;
	val->mVal = mVal + getObjectDataOrig<float>(obj);

	return val;
}

runtimeObjectBase* floatObject::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Sub on const variable");
		return nullptr;
	}
				
	if (!isNumberType(obj))
		return nullptr;

	floatObject *val = new ObjectModule<floatObject>;
	val->mVal = mVal - getObjectDataOrig<float>(obj);

	return val;
}

runtimeObjectBase* floatObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return nullptr;
	}
	
	if (!isNumberType(obj))
		return nullptr;

	floatObject *val = new ObjectModule<floatObject>;
	val->mVal = mVal * getObjectDataOrig<float>(obj);

	return val;
}

runtimeObjectBase* floatObject::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return nullptr;
	}
	
	if (!isNumberType(obj))
		return nullptr;

	floatObject *val = new ObjectModule<floatObject>;
	float divisor = getObjectDataOrig<float>(obj);
	if (fabs(divisor) < 0.0000001)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return nullptr;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* floatObject::SetValue(runtimeObjectBase *obj)
{
	if (!isNumberType(obj))
		return nullptr;
	mVal = getObjectDataOrig<float>(obj);
	return this;
}

runtimeObjectBase* floatObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* floatObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* floatObject::getIndex(int i)
{
	return nullptr;
}

stringObject* floatObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%.6f", mVal);
	return s;
}

bool floatObject::isGreaterThan(const runtimeObjectBase *obj)
{
	if (isNumberType(obj))
	{
		return mVal > getObjectData<floatObject>(obj);
	}
	return false;
}

bool floatObject::isEqual(const runtimeObjectBase *obj)
{
	return isNumberType(obj)
		&& (mVal == getObjectData<floatObject>(obj));
}

///////////////////////////////////////////////////////////////////////////////
