#include "StdAfx.h"
#include "vm.h"

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
		return nullptr;
	}
			
	if (!isNumberType(obj))
		return nullptr;

	doubleObject *val = new ObjectModule<doubleObject>;
	val->mVal = mVal + getObjectDataOrig<double>(obj);

	return val;
}

runtimeObjectBase* doubleObject::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Sub on const variable");
		return nullptr;
	}
				
	if (!isNumberType(obj))
		return nullptr;

	doubleObject *val = new ObjectModule<doubleObject>;
	val->mVal = mVal - getObjectDataOrig<double>(obj);

	return val;
}

runtimeObjectBase* doubleObject::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return nullptr;
	}
	
	if (!isNumberType(obj))
		return nullptr;

	doubleObject *val = new ObjectModule<doubleObject>;
	val->mVal = mVal * getObjectDataOrig<double>(obj);

	return val;
}

runtimeObjectBase* doubleObject::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return nullptr;
	}
	
	if (!isNumberType(obj))
		return nullptr;

	doubleObject *val = new ObjectModule<doubleObject>;
	double divisor = getObjectDataOrig<double>(obj);
	if (fabs(divisor) < 0.000000001)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return nullptr;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* doubleObject::SetValue(runtimeObjectBase *obj)
{
	if (!isNumberType(obj))
		return nullptr;
	mVal = getObjectDataOrig<double>(obj);
	return this;
}

runtimeObjectBase* doubleObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* doubleObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* doubleObject::getIndex(int i)
{
	return nullptr;
}

stringObject* doubleObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%.6f", mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
