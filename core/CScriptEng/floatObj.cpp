#include "StdAfx.h"
#include "vm.h"
#include <math.h>

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
		return NULL;
	}
			
	if (!isNumberType(obj))
		return NULL;

	floatObject *val = new ObjectModule<floatObject>;
	val->mVal = mVal + getObjectDataOrig<float>(obj);

	return val;
}

runtimeObjectBase* floatObject::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Sub on const variable");
		return NULL;
	}
				
	if (!isNumberType(obj))
		return NULL;

	floatObject *val = new ObjectModule<floatObject>;
	val->mVal = mVal - getObjectDataOrig<float>(obj);

	return val;
}

runtimeObjectBase* floatObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return NULL;
	}
	
	if (!isNumberType(obj))
		return NULL;

	floatObject *val = new ObjectModule<floatObject>;
	val->mVal = mVal * getObjectDataOrig<float>(obj);

	return val;
}

runtimeObjectBase* floatObject::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return NULL;
	}
	
	if (!isNumberType(obj))
		return NULL;

	floatObject *val = new ObjectModule<floatObject>;
	float divisor = getObjectDataOrig<float>(obj);
	if (fabs(divisor) < 0.0000001)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return NULL;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* floatObject::SetValue(runtimeObjectBase *obj)
{
	if (!isNumberType(obj))
		return NULL;
	mVal = getObjectDataOrig<float>(obj);
	return this;
}

runtimeObjectBase* floatObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* floatObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* floatObject::getIndex(int i)
{
	return NULL;
}

stringObject* floatObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%.6f", mVal);
	return s;
}

bool floatObject::isGreaterThan(runtimeObjectBase *obj)
{
	if (isNumberType(obj))
	{
		return mVal > getObjectData<floatObject>(obj);
	}
	return false;
}

bool floatObject::isEqual(runtimeObjectBase *obj)
{
	return isNumberType(obj)
		&& (mVal == getObjectData<floatObject>(obj));
}

///////////////////////////////////////////////////////////////////////////////
