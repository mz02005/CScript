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
	std::for_each(mData->begin(), mData->end(),
		[](ArrayTypeInner::reference v) { if (v) v->Release(); });
	mData->clear();
}

uint32_t arrayObject::GetObjectTypeId() const
{
	return DT_array;
}

runtimeObjectBase* arrayObject::Add(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* arrayObject::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* arrayObject::Mul(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* arrayObject::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* arrayObject::SetValue(const runtimeObjectBase *obj)
{
	Clear();
	if (obj->GetObjectTypeId() != DT_array)
		return nullptr;
	const arrayObject *orig = static_cast<const arrayObject*>(obj);
	std::for_each(orig->mData->begin(), orig->mData->end(),
		[this](ArrayTypeInner::reference v)
	{
		mData->push_back(v);
		v->AddRef();
	});
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
	return __super::GetMember(memName);
}

runtimeObjectBase* arrayObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* arrayObject::getIndex(int i)
{
	if (i >= 0 && i < static_cast<int>(mData->size()))
	{
		return (*mData)[i];
	}
	return nullptr;
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
	std::for_each(mData->begin(), mData->end(),
		[s](ArrayTypeInner::reference v)
	{
		stringObject *temp = v->toString();

		(*s->mVal) += temp ? *temp->mVal : "";
		(*s->mVal) += " ";

		if (temp)
		{
			temp->AddRef();
			temp->Release();
		}
	});
	return s;
}

void arrayObject::AddSub(runtime::runtimeObjectBase *o)
{
	o->AddRef();
	mData->push_back(o);
}

///////////////////////////////////////////////////////////////////////////////

uint32_t CreateArrayObj::GetObjectTypeId() const
{
	return runtime::DT_UserTypeBegin;
}

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
	: mArrayObject(nullptr)
{
}

uint32_t Array_addObj::GetObjectTypeId() const
{
	return runtime::DT_UserTypeBegin;
}

runtimeObjectBase* Array_addObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 2)
	{
		SCRIPT_TRACE("array.add(int,obj)\n");
		return nullptr;
	}
	int32_t i = context->GetInt32Param(0);
	runtimeObjectBase *o = context->GetParam(1);

	if (i > static_cast<int32_t>(mArrayObject->mData->size()))
	{
		SCRIPT_TRACE("array.add: index out of range.\n");
		return nullptr;
	}

	if (i < 0)
		i = mArrayObject->mData->size();

	o->AddRef();
	mArrayObject->mData->insert(mArrayObject->mData->begin() + i, o);
	return mArrayObject;
}

///////////////////////////////////////////////////////////////////////////////

Array_deleteObj::Array_deleteObj()
	: mArrayObject(nullptr)
{
}


uint32_t Array_deleteObj::GetObjectTypeId() const
{
	return runtime::DT_UserTypeBegin;
}

runtimeObjectBase* Array_deleteObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 1)
	{
		SCRIPT_TRACE("array.del(int)\n");
		return nullptr;
	}
	int32_t i = context->GetInt32Param(0);

	if (i >= static_cast<int32_t>(mArrayObject->mData->size())
		|| i < 0)
	{
		SCRIPT_TRACE("array.delete: index out of range.\n");
		return nullptr;
	}

	runtimeObjectBase *o = (*mArrayObject->mData)[i];
	mArrayObject->mData->erase(mArrayObject->mData->begin() + i);
	o->Release();
	return mArrayObject;
}

///////////////////////////////////////////////////////////////////////////////

Array_clearObj::Array_clearObj()
	: mArrayObject(nullptr)
{
}

uint32_t Array_clearObj::GetObjectTypeId() const
{
	return runtime::DT_UserTypeBegin;
}

runtimeObjectBase* Array_clearObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 0)
	{
		SCRIPT_TRACE("array.clear()\n");
		return nullptr;
	}

	for (auto iter = mArrayObject->mData->begin(); iter != mArrayObject->mData->end(); iter++)
	{
		(*iter)->Release();
	}
	mArrayObject->mData->clear();
	return mArrayObject;
}
