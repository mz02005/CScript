#include "stdafx.h"
#include "objType.h"
#include "notstd/stringHelper.h"
#include "trace.h"
#include <string.h>

namespace runtime {
	uint32_t NullTypeObject::GetObjectTypeId() const
	{
		return DT_null;
	}

	bool NullTypeObject::isEqual(runtimeObjectBase *obj)
	{
		static runtime::ObjectModule<intObject> nullVal;
		if (obj->GetObjectTypeId() == DT_null)
			return true;

		if (isNumberType(obj) && obj->isEqual(&nullVal))
			return true;

		return false;
	}

	NullTypeObject* NullTypeObject::CreateNullTypeObject()
	{
		return new runtime::ObjectModule<NullTypeObject>;
	}

	/////////////////////////////////////////////////////////////////////////////

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

	bool objTypeObject::isGreaterThan(runtimeObjectBase *obj)
	{
		return mObj ? mObj->isGreaterThan(obj) : false;
	}

	bool objTypeObject::isEqual(runtimeObjectBase *obj)
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

	///////////////////////////////////////////////////////////////////////////

	class MapGetPathObject : public runtime::baseObjDefault
	{
		friend class MapObject;

	private:
		MapObject *mMapObj;

	public:
		MapGetPathObject()
			: mMapObj(nullptr)
		{
		}

		virtual ~MapGetPathObject()
		{
			if (mMapObj)
				mMapObj->Release();
		}

		virtual runtimeObjectBase* doCall(doCallContext *context) override
		{
			do {
				if (context->GetParamCount() != 1)
					break;
				const char *path = context->GetStringParam(0);
				if (!path)
					break;
				const auto parts = notstd::StringHelper::SplitString(path, "/");
				if (parts.empty())
					break;
				const auto *curItem = &mMapObj->mMapData;
				auto s = parts.size();
				s -= 1;
				decltype(s) i = 0;
				for (; i < s; i++)
				{
					auto iter = curItem->find(parts[0]);
					if (iter == curItem->end())
						break;
					auto &nb = iter->second;
					if (nb->GetObjectTypeId() != DT_map)
					{
						SCRIPT_TRACE("MapGetPathObject::doCall: nonmap on path %s\r\n", path);
						break;
					}
					curItem = &reinterpret_cast<MapObject*>(nb)->mMapData;
				}
				if (i != s)
					break;
				auto finalIter = curItem->find(parts.back());
				if (finalIter == curItem->end())
					break;
				return finalIter->second;
			} while (0);
			return runtime::NullTypeObject::CreateNullTypeObject();
		}
	};

	///////////////////////////////////////////////////////////////////////////

	class MapCopyObject : public runtime::baseObjDefault
	{
		friend class MapObject;

	private:
		MapObject *mMapObj;

	public:
		MapCopyObject()
			: mMapObj(nullptr)
		{
		}

		virtual ~MapCopyObject()
		{
			if (mMapObj)
				mMapObj->Release();
		}

		virtual runtimeObjectBase* doCall(doCallContext *context) override
		{
			do {
				if (context->GetParamCount() != 0)
					break;
				MapObject *rr = new ObjectModule<MapObject>;
				rr->CopyFrom(mMapObj);
				return rr;
			} while (0);
			return runtime::NullTypeObject::CreateNullTypeObject();
		}
	};

	///////////////////////////////////////////////////////////////////////////

	class MapGetObject : public runtime::baseObjDefault
	{
		friend class MapObject;

	private:
		MapObject *mMapObj;

	public:
		MapGetObject()
			: mMapObj(nullptr)
		{
		}

		virtual ~MapGetObject()
		{
			if (mMapObj)
				mMapObj->Release();
		}
		
		virtual runtimeObjectBase* doCall(doCallContext *context) override
		{
			do {
				if (context->GetParamCount() != 1)
					break;
				const char *key = context->GetStringParam(0);
				if (!key)
					break;
				auto rr = mMapObj->mMapData.find(key);
				if (rr == mMapObj->mMapData.end())
					break;
				return rr->second;
			} while (0);
			return runtime::NullTypeObject::CreateNullTypeObject();
		}
	};

	///////////////////////////////////////////////////////////////////////////

	class MapSetObject : public runtime::baseObjDefault
	{
		friend class MapObject;

	private:
		MapObject *mMapObj;

	public:
		MapSetObject()
			: mMapObj(nullptr)
		{
		}

		virtual ~MapSetObject()
		{
			if (mMapObj)
				mMapObj->Release();
		}

		virtual runtimeObjectBase* doCall(doCallContext *context) override
		{
			do {
				if (context->GetParamCount() != 2)
					break;
				const char *key = context->GetStringParam(0);
				if (!key)
					break;
				auto val = context->GetParam(1);
				auto ff = mMapObj->mMapData.find(key);
				if (ff != mMapObj->mMapData.end())
				{
					ff->second->Release();
					mMapObj->mMapData.erase(ff);
				}
				if (!mMapObj->mMapData.insert(std::make_pair(key, val)).second)
					break;
				val->AddRef();
			} while (0);
			return runtime::NullTypeObject::CreateNullTypeObject();
		}
	};

	///////////////////////////////////////////////////////////////////////////

	MapObject::~MapObject()
	{
		RemoveAll();
	}

	void MapObject::RemoveAll()
	{
		for (auto &ss : mMapData)
			ss.second->Release();
		mMapData.clear();
	}

	uint32_t MapObject::GetObjectTypeId() const
	{
		return DT_map;
	}

	void MapObject::CopyFrom(const MapObject *obj)
	{
		RemoveAll();
		for (const auto &x : obj->mMapData)
		{
			mMapData[x.first] = x.second;
			x.second->AddRef();
		}
	}

	runtimeObjectBase* MapObject::SetValue(runtimeObjectBase *obj)
	{
		MapObject *toCopy = nullptr;
		if (obj->GetObjectTypeId() == DT_map)
			toCopy = static_cast<MapObject*>(obj);
		else
		{
			if (obj->GetObjectTypeId() == DT_object)
			{
				auto realObj = static_cast<objTypeObject*>(obj)->getInner();
				if (realObj->GetObjectTypeId() == DT_map)
					toCopy = static_cast<MapObject*>(realObj);
			}
		}

		do {
			if (!toCopy)
				break;
			CopyFrom(toCopy);
		} while (0);
		return runtime::NullTypeObject::CreateNullTypeObject();
	}

	runtimeObjectBase* MapObject::GetMember(const char *memName)
	{
		if (!strcmp("get", memName))
		{
			auto r = new runtime::ObjectModule<MapGetObject>;
			r->mMapObj = this;
			AddRef();
			return r;
		}
		if (!strcmp("getPath", memName))
		{
			auto r = new runtime::ObjectModule<MapGetPathObject>;
			r->mMapObj = this;
			AddRef();
			return r;
		}
		if (!strcmp("set", memName))
		{
			auto r = new runtime::ObjectModule<MapSetObject>;
			r->mMapObj = this;
			AddRef();
			return r;
		}
		if (!strcmp("Copy", memName))
		{
			auto r = new runtime::ObjectModule<MapCopyObject>;
			r->mMapObj = this;
			AddRef();
			return r;
		}
		return runtime::baseObjDefault::GetMember(memName);
	}
}
