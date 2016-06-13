#include "stdafx.h"
#include "objType.h"
#include <string.h>

namespace runtime {
	objTypeObject::objTypeObject()
		: mObj(NULL)
	{
	}

	objTypeObject::~objTypeObject()
	{
		if (mObj)
			mObj->Release();
	}

	uint32_t objTypeObject::GetObjectTypeId() const
	{
		return DT_object;
	}

	runtimeObjectBase* objTypeObject::Add(const runtimeObjectBase *obj)
	{
		if (!mObj)
			return mObj;

		return mObj->Add(obj);
	}

	runtimeObjectBase* objTypeObject::Sub(const runtimeObjectBase *obj)
	{
		if (!mObj)
			return mObj;
		return mObj->Sub(mObj);
	}

	runtimeObjectBase* objTypeObject::Mul(const runtimeObjectBase *obj)
	{
		if (!mObj)
			return mObj;

		return mObj->Mul(obj);
	}

	runtimeObjectBase* objTypeObject::Div(const runtimeObjectBase *obj)
	{
		if (!mObj)
			return mObj;

		return mObj->Div(obj);
	}

	runtimeObjectBase* objTypeObject::SetValue(runtimeObjectBase *obj)
	{
		if (mObj)
			mObj->Release();
		mObj = obj;
		mObj->AddRef();
		return this;
	}

	runtimeObjectBase* objTypeObject::GetMember(const char *memName)
	{
		if (!strcmp(memName, "getInner"))
		{
			Object_getInnerObject *a = new runtime::ContainModule<Object_getInnerObject>(this);
			a->mObject = this;
			return a;
		}
		return mObj ? mObj->GetMember(memName) : mObj;
	}

	runtimeObjectBase* objTypeObject::doCall(doCallContext *context)
	{
		return mObj ? mObj->doCall(context) : mObj;
	}

	runtimeObjectBase* objTypeObject::getIndex(int i)
	{
		return mObj ? mObj->getIndex(i) : mObj;
	}

	bool objTypeObject::isGreaterThan(const runtimeObjectBase *obj)
	{
		return mObj ? mObj->isGreaterThan(obj) : false;
	}

	bool objTypeObject::isEqual(const runtimeObjectBase *obj)
	{
		return mObj ? mObj->isEqual(obj) : false;
	}

	stringObject* objTypeObject::toString()
	{
		return mObj ? mObj->toString() : NULL;
	}

	///////////////////////////////////////////////////////////////////////////

	Object_getInnerObject::Object_getInnerObject()
		: mObject(NULL)
	{
	}

	runtimeObjectBase* Object_getInnerObject::doCall(runtime::doCallContext *context)
	{
		return mObject ? mObject->mObj : mObject;
	}
}
