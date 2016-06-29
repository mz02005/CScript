#pragma once
#include "CScriptEng.h"

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

}
