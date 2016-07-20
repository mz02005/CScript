#pragma once
#include "config.h"
#include <string>
#include <assert.h>

namespace runtime {
	class runtimeObjectBase;
	class stringObject;
	class refCounter;

	enum {
		EC_Normal = -100,
	};

	enum
	{
		DT_nullType,
		DT_int8,
		DT_uint8,
		DT_int16,
		DT_uint16,
		DT_int32,
		DT_uint32,
		DT_int64,
		DT_uint64,
		DT_float,
		DT_double,
		DT_string,
		DT_array,
		DT_function,
		DT_object,
		DT_null,

		DT_UserTypeBegin,
	};

	class doCallContext
	{
	public:
		virtual int SetParamCount(uint16_t paramCount) = 0;
		virtual uint32_t GetParamCount() = 0;

		virtual int PushObjectToStack(runtimeObjectBase *obj) = 0;
		virtual runtimeObjectBase* GetParam(uint32_t i) = 0;

		virtual double GetDoubleParam(uint32_t i) = 0;
		virtual float GetFloatParam(uint32_t i) = 0;
		virtual uint32_t GetUint32Param(uint32_t i) = 0;
		virtual int32_t GetInt32Param(uint32_t i) = 0;
		virtual uint16_t GetUint16Param(uint32_t i) = 0;
		virtual int16_t GetInt16Param(uint32_t i) = 0;
		virtual uint8_t GetUint8Param(uint32_t i) = 0;
		virtual int8_t GetInt8Param(uint32_t i) = 0;
		virtual const char* GetStringParam(uint32_t i) = 0;
		virtual const char* GetStringParam(uint32_t i, uint32_t &len) = 0;
		virtual runtimeObjectBase* GetObject(uint32_t i) = 0;

		virtual uint32_t GetArrayParamElemCount(uint32_t i) = 0;
		virtual double GetDoubleElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual float GetFloatElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual uint32_t GetUint32ElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual int32_t GetInt32ElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual uint16_t GetUint16ElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual int16_t GetInt16ElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual uint8_t GetUint8ElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual int8_t GetInt8ElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual const char* GetStringElemOfArrayParam(uint32_t i, uint32_t e) = 0;
		virtual runtimeObjectBase* GetObjectOfArrayParam(uint32_t i, uint32_t e) = 0;

		virtual uint32_t SaveRuntimeStackPosition() = 0;
		virtual void RestoreRuntimeStackPosition(uint32_t oldPos) = 0;
	};

	class refCounter
	{
	public:
		virtual long AddRef() = 0;
		virtual long Release() = 0;
		virtual long ReleaseNotDelete() = 0;
	};

	template <typename T>
	class ObjectModule 
		: public T
	{
	protected:
		long mRef;

	public:
		ObjectModule()
			: mRef(0)
		{
		}

		virtual long AddRef() {
			return ++mRef;
		}

		virtual long Release() {
			long r =  --mRef;
			if (!r) {
				delete this;
				return 0;
			}
			return r;
		}
		
		virtual long ReleaseNotDelete() {
			long r =  --mRef;
			if (!r) {
				return 0;
			}
			return r;
		}
	};

	template <class T>
	class ContainModule : public T
	{
	protected:
		refCounter *mContainer;

	public:
		ContainModule(refCounter *con)
			: mContainer(con)
		{
		}

		virtual long AddRef() {
			return mContainer->AddRef();
		}

		virtual long Release() {
			return mContainer->Release();
		}

		virtual long ReleaseNotDelete() {
			assert(0);
			return -1;
		}
	};
	
	class runtimeObjectBase : public refCounter
	{
	public:
		virtual uint32_t GetObjectTypeId() const = 0;

		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj) = 0;
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj) = 0;
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj) = 0;
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj) = 0;

		// =二元运算
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj) = 0;

		// 处理.操作符（一元的）
		virtual runtimeObjectBase* GetMember(const char *memName) = 0;

		// docall（函数调用一元运算）
		virtual runtimeObjectBase* doCall(doCallContext *context) = 0;

		// getindex（索引访问一元运算）
		virtual runtimeObjectBase* getIndex(int i) = 0;

		// 对象转化为字符串
		virtual stringObject* toString() = 0;

		// 比较
		virtual bool isGreaterThan(runtimeObjectBase *obj) = 0;
		virtual bool isEqual(runtimeObjectBase *obj) = 0;
	};
}
