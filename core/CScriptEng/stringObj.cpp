#include "StdAfx.h"
#include "vm.h"
#include <regex>
#include "arrayType.h"
#include "notstd/stringHelper.h"

namespace runtime {

	///////////////////////////////////////////////////////////////////////////////

	stringObject::stringObject()
		: mVal(new std::string)
	{
	}

	stringObject::~stringObject()
	{
		delete mVal;
	}

	uint32_t stringObject::GetObjectTypeId() const
	{
		return DT_string;
	}

	runtimeObjectBase* stringObject::Add(const runtimeObjectBase *obj)
	{
		if (mIsConst)
		{
			SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
			return NULL;
		}

		runtimeObjectBase *r = NULL;
		uint32_t typeId = obj->GetObjectTypeId();
		if (typeId == DT_string)
		{
			stringObject *val = new ObjectModule<stringObject>;
			*val->mVal = *mVal + *static_cast<const stringObject*>(obj)->mVal;
			r = val;
		}
		else
			return NULL;

		return r;
	}

	runtimeObjectBase* stringObject::Sub(const runtimeObjectBase *obj)
	{
		return NULL;
	}

	runtimeObjectBase* stringObject::Mul(const runtimeObjectBase *obj)
	{
		if (mIsConst)
		{
			SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
			return NULL;
		}

		runtimeObjectBase *r = NULL;
		uint32_t typeId = obj->GetObjectTypeId();
		if (typeId == DT_int32)
		{
			// 通过乘法运算，能够迅速的构造重复的字符串
			stringObject *val = new ObjectModule<stringObject>;
			int count = static_cast<const intObject*>(obj)->mVal;
			for (int32_t i = 0; i < count; i++)
				*val->mVal += *mVal;
			r = val;
		}
		else
			return NULL;

		return r;
	}

	runtimeObjectBase* stringObject::Div(const runtimeObjectBase *obj)
	{
		return NULL;
	}

	runtimeObjectBase* stringObject::SetValue(runtimeObjectBase *obj)
	{
		if (mIsConst)
		{
			SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
			return NULL;
		}
		uint32_t typeId = obj->GetObjectTypeId();

		if (typeId == DT_string)
		{
			*mVal = *static_cast<const stringObject*>(obj)->mVal;
			return this;
		}

		return NULL;
	}

	class String_substrObj : public runtime::baseObjDefault
	{
		friend class stringObject;

	private:
		stringObject *mStringObj;

	public:
		String_substrObj()
			: mStringObj(nullptr)
		{
		}
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			uint32_t c = context->GetParamCount();
			if (c > 2 || c < 1)
			{
				SCRIPT_TRACE("string.substr(int,int)\n"
					"string.substr(int)\n");
				return this;
			}
			int32_t i = context->GetInt32Param(0);
			std::string::size_type sc = -1;
			if (c == 2)
				sc = context->GetUint32Param(1);
			stringObject *r = new runtime::ObjectModule<stringObject>;
			*r->mVal = StringHelper::Mid(*mStringObj->mVal, i, sc);
			return r;
		}
	};

	class String_isMatchObj : public runtime::baseObjDefault
	{
		friend class stringObject;

	private:
		stringObject *mStringObj;

	public:
		String_isMatchObj()
			: mStringObj(nullptr)
		{
		}
		virtual runtimeObjectBase* doCall(doCallContext *context)
		{
			intObject *r = new ObjectModule<intObject>;

			uint32_t c = context->GetParamCount();
			if (c != 1)
				return r;

			const char *s = context->GetStringParam(0);
			if (!s)
				return r;

			try {
				std::regex sm(s);
				r->mVal = std::regex_match(*mStringObj->mVal, sm);
			}
			catch (std::regex_error &e)
			{
				SCRIPT_TRACE("Invalid regex exp: %s(%s)\n", s, e.what());
			}

			return r;
		}
	};

	class String_splitObj : public runtime::baseObjDefault
	{
		friend class stringObject;

	private:
		stringObject *mStringObj;

	public:
		String_splitObj()
			: mStringObj(nullptr)
		{
		}
		virtual runtimeObjectBase* doCall(doCallContext *context)
		{
			arrayObject *a = new ObjectModule<arrayObject>;
			if (context->GetParamCount() != 1)
				return a;
			const char *s = context->GetStringParam(0);
			if (!s)
				return a;
			StringHelper::StringArray sa = StringHelper::SplitString(*mStringObj->mVal, s);
			for (auto iter = sa.begin(); iter != sa.end(); iter++)
			{
				stringObject *ss = new ObjectModule<stringObject>;
				*ss->mVal = *iter;
				a->AddSub(ss);
			}
			return a;
		}
	};

	class String_replaceObj : public runtime::baseObjDefault
	{
		friend class stringObject;

	private:
		stringObject *mStringObj;

	public:
		String_replaceObj()
			: mStringObj(nullptr)
		{
		}
		virtual runtimeObjectBase* doCall(doCallContext *context)
		{
			if (context->GetParamCount() != 2)
				return mStringObj;

			const char *pat = context->GetStringParam(0);
			const char *dest = context->GetStringParam(1);
			if (!pat || !dest)
				return mStringObj;

			StringHelper::Replace(*mStringObj->mVal, pat, dest);

			return mStringObj;
		}
	};

	runtimeObjectBase* stringObject::GetMember(const char *memName)
	{
		if (!strcmp(memName, "len"))
		{
			uintObject *obj = baseTypeObject::CreateBaseTypeObject<uintObject>(false);
			obj->mVal = static_cast<decltype(obj->mVal)>(mVal->size());
			return obj;
		}
		else if (!strcmp(memName, "substr"))
		{
			String_substrObj *o = new runtime::ContainModule<String_substrObj>(this);
			o->mStringObj = this;
			return o;
		}
		else if (!strcmp(memName, "isMatch"))
		{
			String_isMatchObj *o = new ContainModule<String_isMatchObj>(this);
			o->mStringObj = this;
			return o;
		}
		else if (!strcmp(memName, "split"))
		{
			String_splitObj *o = new ContainModule<String_splitObj>(this);
			o->mStringObj = this;
			return o;
		}
		else if (!strcmp(memName, "replace"))
		{
			String_replaceObj *o = new ContainModule<String_replaceObj>(this);
			o->mStringObj = this;
			return o;
		}
		return baseTypeObject::GetMember(memName);
	}

	runtimeObjectBase* stringObject::doCall(doCallContext *context)
	{
		return NULL;
	}

	runtimeObjectBase* stringObject::getIndex(int i)
	{
		std::string::size_type l = mVal->size();
		if (i < 0 || i >= (int)l)
			return NULL;
		intObject *r = new runtime::ObjectModule<intObject>;
		r->mVal = (int)mVal->operator[](i);
		return r;
	}

	stringObject* stringObject::toString()
	{
		//AddRef();
		stringObject *r = new runtime::ObjectModule<stringObject>;
		*r->mVal = *mVal;
		return r;
	}

	bool stringObject::isGreaterThan(runtimeObjectBase *obj)
	{
		return *mVal > getObjectData<stringObject>(obj);
	}

	bool stringObject::isEqual(runtimeObjectBase *obj)
	{
		return *mVal == getObjectData<stringObject>(obj);
	}

	///////////////////////////////////////////////////////////////////////////////
}
