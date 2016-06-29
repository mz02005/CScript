#pragma once
#include "cscriptBase.h"

namespace runtime {
	class CSCRIPTENG_API baseObjDefault : public runtimeObjectBase
	{
	public:
		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);

		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);

		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);

		virtual runtimeObjectBase* getIndex(int i);

		virtual stringObject* toString();

		virtual bool isGreaterThan(runtimeObjectBase *obj);
		virtual bool isEqual(runtimeObjectBase *obj);
	};

	// 外部不能从基本类型派生
	class CSCRIPTENG_API baseTypeObject : public baseObjDefault
	{
	protected:
		bool mIsConst;

	public:
		baseTypeObject();
		virtual ~baseTypeObject();

		template <class T>
		static T* CreateBaseTypeObject(bool isConst)
		{
			T *r = new ObjectModule<T>;
			r->mIsConst = isConst;
			return r;
		}

		virtual runtimeObjectBase* GetMember(const char *memName);
	};

	struct FunctionDesc
	{
		// 函数的字节长度，必须是4的整倍数
		uint32_t len;
		// 函数的名称id，如果为0，表示这个函数是匿名函数
		uint32_t stringId;

		// 函数的参数个数
		uint32_t paramCount;
	};

	class CSCRIPTENG_API intObject : public baseTypeObject
	{
	public:
		typedef int InnerDataType;
		InnerDataType mVal;

	public:
		intObject();

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

		static intObject* CreateIntObject(int v);
	};

	class CSCRIPTENG_API uintObject : public baseTypeObject
	{
	public:
		typedef uint32_t InnerDataType;
		InnerDataType mVal;

	public:
		uintObject();

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
	};

	class CSCRIPTENG_API shortObject : public baseTypeObject
	{
	public:
		typedef short InnerDataType;
		InnerDataType mVal;

	public:
		shortObject();

		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);
		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);
		virtual runtimeObjectBase* getIndex(int i);

		virtual stringObject* toString();
	};

	class CSCRIPTENG_API ushortObject : public baseTypeObject
	{
	public:
		typedef uint16_t InnerDataType;
		InnerDataType mVal;

	public:
		ushortObject();

		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);
		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);
		virtual runtimeObjectBase* getIndex(int i);

		virtual stringObject* toString();
	};

	class CSCRIPTENG_API charObject : public baseTypeObject
	{
	public:
		typedef int8_t InnerDataType;
		InnerDataType mVal;

	public:
		charObject();

		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);
		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);
		virtual runtimeObjectBase* getIndex(int i);

		virtual stringObject* toString();
	};

	class CSCRIPTENG_API byteObject : public baseTypeObject
	{
	public:
		typedef uint8_t InnerDataType;
		InnerDataType mVal;

	public:
		byteObject();

		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);
		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);
		virtual runtimeObjectBase* getIndex(int i);

		virtual stringObject* toString();
	};

	class CSCRIPTENG_API floatObject : public baseTypeObject
	{
	public:
		typedef float InnerDataType;
		InnerDataType mVal;

	public:
		floatObject();

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
	};

	class CSCRIPTENG_API doubleObject : public baseTypeObject
	{
	public:
		typedef double InnerDataType;
		InnerDataType mVal;

	public:
		doubleObject();

		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);
		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);
		virtual runtimeObjectBase* getIndex(int i);

		virtual stringObject* toString();
	};

	class CSCRIPTENG_API stringObject : public baseTypeObject
	{
	public:
		typedef const char* InnerDataType;
		std::string *mVal;

	public:
		stringObject();
		virtual ~stringObject();

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
	};

	class runtimeContext;
	class CSCRIPTENG_API FunctionObject : public baseTypeObject
	{
	private:
		FunctionDesc mFuncDesc;
		uint32_t *mInstHead, *mInstTail;
		runtimeContext *mContext;
		std::string *mFuncName;

	public:
		FunctionObject();
		virtual ~FunctionObject();

		const FunctionDesc* GetDesc() const { return &mFuncDesc; }
		const std::string& GetFuncName() const { return *mFuncName; }

		bool ReadFuncHeader(uint32_t *codeHeader, uint32_t *codeTail,
			uint32_t off, runtimeContext *context);

		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* Add(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* Div(const runtimeObjectBase *obj);
		virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj);
		virtual runtimeObjectBase* GetMember(const char *memName);
		virtual runtimeObjectBase* doCall(doCallContext *context);
		virtual runtimeObjectBase* getIndex(int i);
		virtual stringObject* toString();
		virtual bool isGreaterThan(runtimeObjectBase *obj);
		virtual bool isEqual(runtimeObjectBase *obj);
	};

	// 一些辅助函数
	inline bool isNumberType(const runtimeObjectBase *o)
	{
		uint32_t typeId = o->GetObjectTypeId();
		return typeId >= DT_int8 && typeId <= DT_double;
	}

	inline bool isIntegerType(const runtimeObjectBase *o)
	{
		uint32_t typeId = o->GetObjectTypeId();
		return typeId >= DT_int8 && typeId <= DT_int32;
	}

	template <typename T>
	T getObjectDataOrig(const runtimeObjectBase *o)
	{
		T r = 0;
		switch (o->GetObjectTypeId())
		{
		case DT_int8:
			r = (T)static_cast<const charObject*>(o)->mVal;
			break;
		case DT_uint8:
			r = (T)static_cast<const byteObject*>(o)->mVal;
			break;
		case DT_int16:
			r = (T)static_cast<const shortObject*>(o)->mVal;
			break;
		case DT_uint16:
			r = (T)static_cast<const ushortObject*>(o)->mVal;
			break;
		case DT_int32:
			r = (T)static_cast<const intObject*>(o)->mVal;
			break;
		case DT_uint32:
			r = (T)static_cast<const uintObject*>(o)->mVal;
			break;
		case DT_int64:
			break;
		case DT_uint64:
			break;
		case DT_float:
			r = (T)static_cast<const floatObject*>(o)->mVal;
			break;
		case DT_double:
			r = (T)static_cast<const doubleObject*>(o)->mVal;
			break;
		default:
			break;
		}
		return r;
	}

	template <typename T>
	typename T::InnerDataType getObjectData(const runtimeObjectBase *o)
	{
		typedef typename T::InnerDataType ReturnType;
		ReturnType r = (ReturnType)0;
		switch (o->GetObjectTypeId())
		{
		case DT_int8:
			r = (ReturnType)static_cast<const charObject*>(o)->mVal;
			break;
		case DT_uint8:
			r = (ReturnType)static_cast<const byteObject*>(o)->mVal;
			break;
		case DT_int16:
			r = (ReturnType)static_cast<const shortObject*>(o)->mVal;
			break;
		case DT_uint16:
			r = (ReturnType)static_cast<const ushortObject*>(o)->mVal;
			break;
		case DT_int32:
			r = (ReturnType)static_cast<const intObject*>(o)->mVal;
			break;
		case DT_uint32:
			r = (ReturnType)static_cast<const uintObject*>(o)->mVal;
			break;
		case DT_int64:
			break;
		case DT_uint64:
			break;
		case DT_float:
			r = (ReturnType)static_cast<const floatObject*>(o)->mVal;
			break;
		case DT_double:
			r = (ReturnType)static_cast<const doubleObject*>(o)->mVal;
			break;
		default:
			break;
		}
		return r;
	}

	template <>
	inline stringObject::InnerDataType getObjectData<stringObject>(const runtimeObjectBase *o)
	{
		typedef stringObject::InnerDataType ReturnType;
		const static char empty[] = "";
		if (o->GetObjectTypeId() == DT_string)
			return static_cast<const stringObject*>(o)->mVal->c_str();
		return (ReturnType)empty;
	}
}
