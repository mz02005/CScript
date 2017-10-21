#include "stdafx.h"
#include "vm.h"
#include <math.h>

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

doubleObject::doubleObject()
	: mVal(0)
{
}

uint32_t doubleObject::GetObjectTypeId() const
{
	return DT_double;
}

runtimeObjectBase* doubleObject::Add(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Add on const variable");
		return NULL;
	}
			
	if (!isNumberType(obj))
		return NULL;

	doubleObject *val = new ObjectModule<doubleObject>;
	val->mVal = mVal + getObjectDataOrig<double>(obj);

	return val;
}

runtimeObjectBase* doubleObject::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Sub on const variable");
		return NULL;
	}
				
	if (!isNumberType(obj))
		return NULL;

	doubleObject *val = new ObjectModule<doubleObject>;
	val->mVal = mVal - getObjectDataOrig<double>(obj);

	return val;
}

runtimeObjectBase* doubleObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return NULL;
	}
	
	if (!isNumberType(obj))
		return NULL;

	doubleObject *val = new ObjectModule<doubleObject>;
	val->mVal = mVal * getObjectDataOrig<double>(obj);

	return val;
}

runtimeObjectBase* doubleObject::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return NULL;
	}
	
	if (!isNumberType(obj))
		return NULL;

	doubleObject *val = new ObjectModule<doubleObject>;
	double divisor = getObjectDataOrig<double>(obj);
	if (fabs(divisor) < 0.000000001)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return NULL;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* doubleObject::SetValue(runtimeObjectBase *obj)
{
	if (!isNumberType(obj))
		return NULL;
	mVal = getObjectDataOrig<double>(obj);
	return this;
}

runtimeObjectBase* doubleObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* doubleObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* doubleObject::getIndex(int i)
{
	return NULL;
}

stringObject* doubleObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	notstd::StringHelper::Format(*s->mVal, "%.6f", mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
