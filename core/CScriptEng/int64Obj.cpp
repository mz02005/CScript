#include "stdafx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

int64Object::int64Object()
	: mVal(0)
{
}

uint32_t int64Object::GetObjectTypeId() const
{
	return DT_int64;
}

runtimeObjectBase* int64Object::Add(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	int64Object *val = new ObjectModule<int64Object>;
	val->mVal = mVal + getObjectDataOrig<int64_t>(obj);
	return val;
}

runtimeObjectBase* int64Object::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("sub on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	int64Object *val = new ObjectModule<int64Object>;
	val->mVal = mVal - getObjectDataOrig<int64_t>(obj);
	return val;
}

runtimeObjectBase* int64Object::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	int64Object *val = new ObjectModule<int64Object>;
	val->mVal = mVal * getObjectDataOrig<int64_t>(obj);
	return val;
}

runtimeObjectBase* int64Object::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	int64Object *val = new ObjectModule<int64Object>;
	int64_t divisor = getObjectDataOrig<int64_t>(obj);
	if (!divisor)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return NULL;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* int64Object::SetValue(runtimeObjectBase *obj)
{
	if (!isNumberType(obj))
		return NULL;
	mVal = getObjectDataOrig<int64_t>(obj);
	return this;
}

runtimeObjectBase* int64Object::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* int64Object::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* int64Object::getIndex(int i)
{
	return nullptr;
}

stringObject* int64Object::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	notstd::StringHelper::Format(*s->mVal, "%lld", mVal);
	return s;
}

bool int64Object::isGreaterThan(runtimeObjectBase *obj)
{
	if (isNumberType(obj))
	{
		return mVal > getObjectData<int64Object>(obj);
	}
	return false;
}

bool int64Object::isEqual(runtimeObjectBase *obj)
{
	return isNumberType(obj)
		&& (mVal == getObjectData<int64Object>(obj));
}

int64Object* int64Object::CreateInt64Object(int64_t v)
{
	int64Object *r = new runtime::ObjectModule<int64Object>;
	r->mVal = v;
	return r;
}

///////////////////////////////////////////////////////////////////////////////
