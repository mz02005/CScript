#include "stdafx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

uint64Object::uint64Object()
	: mVal(0)
{
}

uint32_t uint64Object::GetObjectTypeId() const
{
	return DT_int64;
}

runtimeObjectBase* uint64Object::Add(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	uint64Object *val = new ObjectModule<uint64Object>;
	val->mVal = mVal + getObjectDataOrig<uint64_t>(obj);
	return val;
}

runtimeObjectBase* uint64Object::Sub(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("sub on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	uint64Object *val = new ObjectModule<uint64Object>;
	val->mVal = mVal - getObjectDataOrig<uint64_t>(obj);
	return val;
}

runtimeObjectBase* uint64Object::Mul(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Mul on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	uint64Object *val = new ObjectModule<uint64Object>;
	val->mVal = mVal * getObjectDataOrig<uint64_t>(obj);
	return val;
}

runtimeObjectBase* uint64Object::Div(const runtimeObjectBase *obj)
{
	if (mIsConst)
	{
		SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Div on const variable");
		return NULL;
	}

	if (!isNumberType(obj))
		return NULL;

	uint64Object *val = new ObjectModule<uint64Object>;
	uint64_t divisor = getObjectDataOrig<uint64_t>(obj);
	if (!divisor)
	{
		SCRIPT_TRACE("Divided by zero\n");
		return NULL;
	}
	val->mVal = mVal / divisor;
	return val;
}

runtimeObjectBase* uint64Object::SetValue(runtimeObjectBase *obj)
{
	if (!isNumberType(obj))
		return NULL;
	mVal = getObjectDataOrig<uint64_t>(obj);
	return this;
}

runtimeObjectBase* uint64Object::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* uint64Object::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* uint64Object::getIndex(int i)
{
	return nullptr;
}

stringObject* uint64Object::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	notstd::StringHelper::Format(*s->mVal, "%lld", mVal);
	return s;
}

bool uint64Object::isGreaterThan(runtimeObjectBase *obj)
{
	if (isNumberType(obj))
	{
		return mVal > getObjectData<uint64Object>(obj);
	}
	return false;
}

bool uint64Object::isEqual(runtimeObjectBase *obj)
{
	return isNumberType(obj)
		&& (mVal == getObjectData<uint64Object>(obj));
}

uint64Object* uint64Object::CreateUint64Object(uint64_t v)
{
	uint64Object *r = new runtime::ObjectModule<uint64Object>;
	r->mVal = v;
	return r;
}

///////////////////////////////////////////////////////////////////////////////
