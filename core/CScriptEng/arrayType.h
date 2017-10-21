#pragma once
#include "rtTypes.h"

namespace runtime {
	class CreateArrayObj;
	class CSCRIPTENG_API arrayObject : public baseTypeObject
	{
		friend class CreateArrayObj;
	public:
		typedef std::vector<runtimeObjectBase*> ArrayTypeInner;
		ArrayTypeInner *mData;

	public:
		arrayObject();
		virtual ~arrayObject();

		void Clear();

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

		void AddSub(runtime::runtimeObjectBase *o);
		runtime::runtimeObjectBase* GetSub(uint32_t i);
	};

	class CreateArrayObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};
}
