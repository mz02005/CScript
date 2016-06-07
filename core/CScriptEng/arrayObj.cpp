#include "stdAfx.h"
#include "arrayType.h"

using namespace runtime;

arrayObject::arrayObject()
	: mData(new ArrayTypeInner)
{
}

arrayObject::~arrayObject()
{
	Clear();
	delete mData;
}

void arrayObject::Clear()
{
	for (ArrayTypeInner::iterator iter = mData->begin();
		iter != mData->end(); iter++)
	{
		if (*iter)
			(*iter)->Release();
	}
	mData->clear();
}

uint32_t arrayObject::GetObjectTypeId() const
{
	return DT_array;
}

runtimeObjectBase* arrayObject::Add(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* arrayObject::Sub(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* arrayObject::Mul(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* arrayObject::Div(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* arrayObject::SetValue(runtimeObjectBase *obj)
{
	Clear();
	if (obj->GetObjectTypeId() != DT_array)
		return NULL;
	const arrayObject *orig = static_cast<const arrayObject*>(obj);
	for (ArrayTypeInner::iterator iter = orig->mData->begin();
		iter != orig->mData->end(); iter++)
	{
		mData->push_back(*iter);
		(*iter)->AddRef();
	}
	return this;
}

runtimeObjectBase* arrayObject::GetMember(const char *memName)
{
	if (!strcmp(memName, "size"))
	{
		intObject *l = new runtime::ObjectModule<intObject>;
		l->mVal = static_cast<int>(mData->size());
		return l;
	}
	else if (!strcmp(memName, "add"))
	{
		Array_addObj *a = new runtime::ContainModule<Array_addObj>(this);
		a->mArrayObject = this;
		return a;
	}
	else if (!strcmp(memName, "del"))
	{
		Array_deleteObj *a = new runtime::ContainModule<Array_deleteObj>(this);
		a->mArrayObject = this;
		return a;
	}
	else if (!strcmp(memName, "clear"))
	{
		Array_clearObj *a = new runtime::ContainModule<Array_clearObj>(this);
		a->mArrayObject = this;
		return a;
	}
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* arrayObject::doCall(doCallContext *context)
{
	return NULL;
}

runtimeObjectBase* arrayObject::getIndex(int i)
{
	if (i >= 0 && i < static_cast<int>(mData->size()))
	{
		return (*mData)[i];
	}
	return NULL;
}

bool arrayObject::isGreaterThan(const runtimeObjectBase *obj)
{
	return false;
}

bool arrayObject::isEqual(const runtimeObjectBase *obj)
{
	if (obj->GetObjectTypeId() != DT_array)
		return false;

	const arrayObject *a = static_cast<const arrayObject*>(obj);
	if (a->mData->size() != mData->size())
		return false;

	size_t t = mData->size();
	for (size_t s = 0; s < t; s++)
	{
		if (!(*mData)[s]->isEqual((*a->mData)[s]))
			return false;
	}

	return true;
}

stringObject* arrayObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	for (ArrayTypeInner::iterator iter = mData->begin();
		iter != mData->end(); iter++)
	{
		runtimeObjectBase *v = *iter;
		stringObject *temp = v->toString();

		(*s->mVal) += temp ? *temp->mVal : "";
		(*s->mVal) += " ";

		if (temp)
		{
			temp->AddRef();
			temp->Release();
		}
	}
	return s;
}

void arrayObject::AddSub(runtime::runtimeObjectBase *o)
{
	o->AddRef();
	mData->push_back(o);
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* CreateArrayObj::doCall(runtime::doCallContext *context)
{
	uint32_t paramCount = context->GetParamCount();

	runtime::arrayObject *a = new runtime::ObjectModule<runtime::arrayObject>;
	for (uint32_t s = 0; s < paramCount; s ++)
	{
		runtimeObjectBase *o = context->GetParam(s);
		o->AddRef();
		a->mData->push_back(o);
	}

	return a;
}

///////////////////////////////////////////////////////////////////////////////

Array_addObj::Array_addObj()
	: mArrayObject(NULL)
{
}

runtimeObjectBase* Array_addObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 2)
	{
		SCRIPT_TRACE("array.add(int,obj)\n");
		return NULL;
	}
	int32_t i = context->GetInt32Param(0);
	runtimeObjectBase *o = context->GetParam(1);

	if (i > static_cast<int32_t>(mArrayObject->mData->size()))
	{
		SCRIPT_TRACE("array.add: index out of range.\n");
		return NULL;
	}

	if (i < 0)
		i = mArrayObject->mData->size();

	o->AddRef();
	mArrayObject->mData->insert(mArrayObject->mData->begin() + i, o);
	return mArrayObject;
}

///////////////////////////////////////////////////////////////////////////////

Array_deleteObj::Array_deleteObj()
	: mArrayObject(NULL)
{
}

runtimeObjectBase* Array_deleteObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 1)
	{
		SCRIPT_TRACE("array.del(int)\n");
		return NULL;
	}
	int32_t i = context->GetInt32Param(0);

	if (i >= static_cast<int32_t>(mArrayObject->mData->size())
		|| i < 0)
	{
		SCRIPT_TRACE("array.delete: index out of range.\n");
		return NULL;
	}

	runtimeObjectBase *o = (*mArrayObject->mData)[i];
	mArrayObject->mData->erase(mArrayObject->mData->begin() + i);
	o->Release();
	return mArrayObject;
}

///////////////////////////////////////////////////////////////////////////////

Array_clearObj::Array_clearObj()
	: mArrayObject(NULL)
{
}

runtimeObjectBase* Array_clearObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 0)
	{
		SCRIPT_TRACE("array.clear()\n");
		return NULL;
	}

	for (auto iter = mArrayObject->mData->begin(); iter != mArrayObject->mData->end(); iter++)
	{
		(*iter)->Release();
	}
	mArrayObject->mData->clear();
	return mArrayObject;
}
