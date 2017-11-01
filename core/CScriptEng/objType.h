#pragma once
#include "rtTypes.h"

namespace runtime {
	// null¿‡–Õ
	class CSCRIPTENG_API NullTypeObject :public baseTypeObject
	{
	public:
		virtual uint32_t GetObjectTypeId() const;
		virtual bool isEqual(runtimeObjectBase *obj);

		static NullTypeObject* CreateNullTypeObject();
	};

	class Object_getInnerObject;
	class CSCRIPTENG_API objTypeObject : public baseTypeObject
	{
		friend class Object_getInnerObject;
	private:
		runtimeObjectBase *mObj;

	public:
		objTypeObject();
		virtual ~objTypeObject();

		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);
		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);
		virtual runtimeObjectBase* getIndex(int i);

		virtual bool isGreaterThan(runtimeObjectBase *obj);
		virtual bool isEqual(runtimeObjectBase *obj);

		virtual stringObject* toString();

		runtimeObjectBase* getInner() { return mObj; }
	};
	
	class Object_getInnerObject : public runtime::baseObjDefault
	{
		friend class objTypeObject;

	private:
		objTypeObject *mObject;

	public:
		Object_getInnerObject();
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class MapSetObject;
	class MapGetObject;
	class MapGetPathObject;
	class MapCopyObject;
	class MapObject : public runtime::baseObjDefault
	{
		friend class runtimeContext;
		friend class MapSetObject;
		friend class MapGetObject;
		friend class MapGetPathObject;
		friend class MapCopyObject;

	private:
		std::map<std::string, runtime::runtimeObjectBase*> mMapData;

		void RemoveAll();
		void CopyFrom(const MapObject *obj);

	public:
		virtual ~MapObject();
		virtual runtimeObjectBase* GetMember(const char *memName) override;
		virtual uint32_t GetObjectTypeId() const override;
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj) override;
	};
}
