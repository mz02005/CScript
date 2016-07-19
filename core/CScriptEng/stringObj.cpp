#include "stdafx.h"
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

	class String_formatObj : public runtime::baseObjDefault
	{
		friend class stringObject;

	private:
		stringObject *mStringObj;

		struct StringFormatInfo {
			// 是否是左对齐
			uint32_t alignLeft : 1;
			// 填充字符，缺省为' '
			uint32_t fillChar : 8;

			// 小数点位数
			uint32_t prec : 7;

			// 长度
			uint32_t len : 8;
		};

		inline std::string GetFloatString(float v, StringFormatInfo *sfi)
		{
			std::string s, r;
			//									f
			if (sfi->len)
				notstd::StringHelper::Format(s, "%%%c%d.%df", (char)sfi->fillChar,
				(int)sfi->len, (int)sfi->prec);
			else
				notstd::StringHelper::Format(s, "%%.%df", (int)sfi->prec);
			return notstd::StringHelper::Format(r, s.c_str(), v);
		}

		inline void AppendString(const std::string &s, StringFormatInfo *sfi)
		{
			size_t ll = s.size();
			if (sfi->alignLeft)
			{
				mStringObj->mVal->append(s);
				if (ll < sfi->len)
				{
					int x = 0, tf = sfi->len - ll;
					for (; x < tf; x++)
						*mStringObj->mVal += sfi->fillChar;
				}
			}
			else
			{
				if (ll < sfi->len)
				{
					int x = 0, tf = sfi->len - ll;
					for (; x < tf; x++)
						*mStringObj->mVal += sfi->fillChar;
				}
				mStringObj->mVal->append(s);
			}
		}

	public:
		String_formatObj()
			: mStringObj(nullptr)
		{
		}

		// 实现format函数
		// 本format实现了一部分《Format Specification Syntax: printf and wprintf Functions》
		// 规定的格式信息，所有和标准不符合的部分都以本函数的实际输出为准
		virtual runtimeObjectBase* doCall(doCallContext *context)
		{
			runtime::NullTypeObject *nullRet = runtime::NullTypeObject::CreateNullTypeObject();
			const char *p1;
			union
			{
				const char *ss;
				uint32_t u32Val;
			};
			if (context->GetParamCount() < 1
				|| !(p1 = context->GetStringParam(0)))
			{
				SCRIPT_TRACE("string.format(formatString, ...\n");
				return nullRet;
			}

			StringFormatInfo formatInfo;

			bool flagOK;
			bool widthOK;
			bool precisionOK;

			uint32_t paramCount = 1;
			mStringObj->mVal->clear();
			for (; *p1 != 0; p1++)
			{
				switch (*p1)
				{
				case '%':
					if (*++p1 == 0)
					{
						SCRIPT_TRACE("string.format: more information expected.\n");
						return nullRet;
					}
					if (*p1 != '%' && context->GetParamCount() < ++paramCount)
					{
						SCRIPT_TRACE("string.format: parameter count out of range.\n");
						return nullRet;
					}

					memset(&formatInfo, 0, sizeof(formatInfo));
					formatInfo.fillChar = ' ';

					flagOK = false;
					widthOK = false;
					precisionOK = false;
					for (; *p1 != 0;)
					{
						if (!flagOK)
						{
							if (*p1 == '-')
							{
								formatInfo.alignLeft = 1;
								char c = *(p1 + 1);
								if (c == ' ' || c == '0') {
									formatInfo.fillChar = c;
									p1 += 2;
								}
								else
									p1++;
							}
							else if (*p1 == '+')
							{
								p1++;
							}
							else if (*p1 == '0')
							{
								formatInfo.fillChar = '0';
								p1++;
							}
						}
						if (!widthOK)
						{
							if (*p1 >= '1' && *p1 <= '9')
							{
								flagOK = true;
								widthOK = true;
								uint32_t width = *p1 - '0';
								while (*++p1 && *p1 >= '0' && *p1 <= '9')
								{
									width *= 10;
									width += *p1 - '0';
								}
								formatInfo.len = width;
								flagOK = true;
							}
							else if (*p1 == '*')
							{
								flagOK = true;
								widthOK = true;

								if (context->GetParamCount() < ++paramCount)
								{
									SCRIPT_TRACE("string.format: more information expected.\n");
									return nullRet;
								}
								try {
									formatInfo.len = context->GetInt32Param(paramCount - 1);
								}
								catch (...)
								{
									SCRIPT_TRACE("string.format: integer parameter expected for item length.\n");
									return nullRet;
								}
							}
						}
						if (!precisionOK)
						{
							if (*p1 == '.')
							{
								flagOK = true;
								widthOK = true;
								precisionOK = true;

								uint32_t prec = 0;
								while (*++p1 && *p1 >= '0' && *p1 <= '9')
								{
									prec *= 10;
									prec += *p1 - '0';
								}
								formatInfo.prec = prec;
							}
						}

						if (*p1 == '%')
						{
							*mStringObj->mVal += '%';
							p1++;
							break;
						}
						else if (*p1 == 's')
						{
							if ((ss = context->GetStringParam(paramCount - 1)) == nullptr)
							{
								SCRIPT_TRACE("string.format: string parameter expected at position %u\n", paramCount - 1);
								return nullRet;
							}
							AppendString(ss, &formatInfo);
							break;
						}
						else if (*p1 == 'd')
						{
							try {
								u32Val = context->GetInt32Param(paramCount - 1);
							}
							catch (...)
							{
								SCRIPT_TRACE("string.format: integer parameter expected at position %u\n", paramCount - 1);
								return nullRet;
							}
							std::string v = std::to_string(u32Val);
							AppendString(v, &formatInfo);
							break;
						}
						else if (*p1 == 'f')
						{
							float f;
							try {
								f = context->GetFloatParam(paramCount - 1);
							}
							catch (...)
							{
								SCRIPT_TRACE("string.format: float parameter expected at position %u\n", paramCount - 1);
								return nullRet;
							}
							std::string v = GetFloatString(f, &formatInfo);
							AppendString(v, &formatInfo);
							break;
						}
						else if (*p1 == 'X')
						{
							int v;
							try
							{
								v = context->GetInt32Param(paramCount - 1);
							}
							catch (...)
							{
								SCRIPT_TRACE("string.format: integer parameter expected at position %u.\n", paramCount - 1);
								return nullRet;
							}
							std::string ss;
							notstd::StringHelper::Format(ss, "%X", v);
							AppendString(ss, &formatInfo);
							break;
						}
						else
						{
							SCRIPT_TRACE("string.format: invalid format type %c\n", *p1);
							return nullRet;
						}
					}
					break;

				default:
					*mStringObj->mVal += *p1;
					break;
				}
			}

			delete nullRet;
			return mStringObj;
		}
	};

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
			*r->mVal = notstd::StringHelper::Mid(*mStringObj->mVal, i, sc);
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
			notstd::StringHelper::StringArray sa = notstd::StringHelper::SplitString(*mStringObj->mVal, s);
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

			notstd::StringHelper::Replace(*mStringObj->mVal, pat, dest);

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
		else if (!strcmp(memName, "format"))
		{
			String_formatObj *o = new runtime::ContainModule<String_formatObj>(this);
			o->mStringObj = this;
			return o;
		}
		else if (!strcmp(memName, "substr"))
		{
			String_substrObj *o = new runtime::ContainModule<String_substrObj>(this);
			o->mStringObj = this;
			return o;
		}
		else if (!strcmp(memName, "split"))
		{
			String_splitObj *o = new ContainModule<String_splitObj>(this);
			o->mStringObj = this;
			return o;
		}
		else if (!strcmp(memName, "isMatch"))
		{
			String_isMatchObj *o = new ContainModule<String_isMatchObj>(this);
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
		return obj->GetObjectTypeId() == DT_string ? *mVal > getObjectData<stringObject>(obj) : false;
	}

	bool stringObject::isEqual(runtimeObjectBase *obj)
	{
		return obj->GetObjectTypeId() == DT_string ? *mVal == getObjectData<stringObject>(obj) : false;
	}

	///////////////////////////////////////////////////////////////////////////////
}
