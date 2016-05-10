#pragma once

namespace runtime {
	class runtimeObjectBase;
	class stringObject;
	class refCounter;

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

		DT_UserTypeBegin,
	};

	class doCallContext
	{
	public:
		virtual uint32_t GetParamCount() = 0;

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
	};

	class refCounter
	{
	public:
		virtual LONG AddRef() = 0;
		virtual LONG Release() = 0;
		virtual LONG ReleaseNotDelete() = 0;
	};

	template <typename T>
	class ObjectModule 
		: public T
	{
	protected:
		LONG mRef;

	public:
		ObjectModule()
			: mRef(0)
		{
		}

		virtual LONG AddRef() {
			return ++mRef;
		}

		virtual LONG Release() {
			LONG r =  --mRef;
			if (!r) {
				delete this;
				return 0;
			}
			return r;
		}
		
		virtual LONG ReleaseNotDelete() {
			LONG r =  --mRef;
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
		LONG mRef;
		refCounter *mContainer;

	public:
		ContainModule(refCounter *con)
			: mContainer(con)
			, mRef(0)
		{
		}

		virtual LONG AddRef() {
			mContainer->AddRef();
			return ++mRef;
		}

		virtual LONG Release() {
			mContainer->Release();
			LONG r =  --mRef;
			if (!r) {
				delete this;
				return 0;
			}
			return r;
		}

		virtual LONG ReleaseNotDelete() {
			mContainer->Release();
			LONG r =  --mRef;
			if (!r) {
				return 0;
			}
			return r;
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
		virtual runtimeObjectBase* SetValue(const runtimeObjectBase *obj) = 0;

		// 处理.操作符（一元的）
		virtual runtimeObjectBase* GetMember(const char *memName) = 0;

		// docall（函数调用一元运算）
		virtual runtimeObjectBase* doCall(doCallContext *context) = 0;

		// getindex（索引访问一元运算）
		virtual runtimeObjectBase* getIndex(int i) = 0;

		// 对象转化为字符串
		virtual stringObject* toString() = 0;

		// 比较
		virtual bool isGreaterThan(const runtimeObjectBase *obj) = 0;
		virtual bool isEqual(const runtimeObjectBase *obj) = 0;
	};
}
