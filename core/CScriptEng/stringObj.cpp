#include "stdafx.h"
#include "vm.h"
#include <regex>
#include "arrayType.h"
#include "notstd/stringHelper.h"
#include "BufferObject.h"

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
		else if (typeId == DT_null)
		{
			return this;
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
		NullTypeObject *nullRet = NullTypeObject::CreateNullTypeObject();
		if (mIsConst)
		{
			SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("add on const variable");
			return nullRet;
		}

		runtimeObjectBase *r = NULL;
		if (isIntegerType(obj))
		{
			int32_t c = getObjectDataOrig<int32_t>(obj);
			if (c > 0)
			{
				// 通过乘法运算，能够迅速的构造重复的字符串
				stringObject *val = new ObjectModule<stringObject>;
				int count = static_cast<const intObject*>(obj)->mVal;
				for (int32_t i = 0; i < count; i++)
					val->mVal->append(mVal->c_str(), mVal->size());
				r = val;
				delete nullRet;
				return r;
			}
		}
		SCRIPT_TRACE("string.*: string count must larger than 0.\n");
		return nullRet;
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

			enum FormatParseStage
			{
				FPS_FLAG,
				FPS_WIDTH,
				FPS_PRECISION,
				//FPS_SIZE,
				FPS_TYPE,
			};
			FormatParseStage parseStage;
			//bool flagOK;
			//bool widthOK;
			//bool precisionOK;

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

					parseStage = FPS_FLAG;
					for (; *p1 != 0;)
					{
						if (parseStage <= FPS_FLAG)
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
						if (parseStage <= FPS_WIDTH)
						{
							if (*p1 >= '1' && *p1 <= '9')
							{
								uint32_t width = *p1 - '0';
								while (*++p1 && *p1 >= '0' && *p1 <= '9')
								{
									width *= 10;
									width += *p1 - '0';
								}
								formatInfo.len = width;
								parseStage = FPS_PRECISION;
							}
							else if (*p1 == '*')
							{
								if (context->GetParamCount() < ++paramCount)
								{
									SCRIPT_TRACE("string.format: more information expected.\n");
									return nullRet;
								}
								try {
									formatInfo.len = context->GetInt32Param(paramCount - 2);
								}
								catch (...)
								{
									SCRIPT_TRACE("string.format: integer parameter expected for item length.\n");
									return nullRet;
								}
								p1++;
								parseStage = FPS_PRECISION;
							}
						}
						if (parseStage <= FPS_PRECISION)
						{
							if (*p1 == '.')
							{
								uint32_t prec = 0;
								while (*++p1 && *p1 >= '0' && *p1 <= '9')
								{
									prec *= 10;
									prec += *p1 - '0';
								}
								formatInfo.prec = prec;
								parseStage = FPS_TYPE;
							}
						}

						if (*p1 == '%')
						{
							*mStringObj->mVal += '%';
							p1++;
							break;
						}
						else if (*p1 == 'c')
						{
							runtimeObjectBase *o = context->GetParam(paramCount - 1);
							uint32_t ot = o->GetObjectTypeId();
							std::string v;
							if (ot == DT_string)
							{
								v.append(static_cast<stringObject*>(o)->mVal->c_str(), 1);
							}
							else if (isIntegerType(o))
							{
								int iv = getObjectDataOrig<int>(o);
								if (iv < 0)
								{
									SCRIPT_TRACE("string.format: %%c type switch must use integer parameter larger or equal to 0\n");
									return nullRet;
								}
								v = std::to_string(iv);
							}
							else
							{
								SCRIPT_TRACE("string.format: %%c type switch muse use integer or string parameter.\n");
								return nullRet;
							}
							AppendString(v, &formatInfo);
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
						else if (*p1 == 'X' || *p1 == 'x')
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
							notstd::StringHelper::Format(ss, *p1 == 'X' ? "%X" : "%x", v);
							AppendString(ss, &formatInfo);
							break;
						}
						else if (*p1 == 'u')
						{
							int v;
							try
							{
								v = context->GetUint32Param(paramCount - 1);
							}
							catch (...)
							{
								SCRIPT_TRACE("string.format: unsigned integer parameter expected at position %u.\n", paramCount - 1);
								return nullRet;
							}
							AppendString(notstd::StringHelper::Format("%u", v), &formatInfo);
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

	class StringCodecObj : public runtime::baseObjDefault
	{
		friend class stringObject;

	private:
		enum {
			CC_NULL,
			CC_MBCS2UNICODE,
			CC_UNICODE2MBCS,
			CC_UTF82UNICODE,
			CC_UNICODE2UTF8,
		} mConvType;
		stringObject *mStringObj;

	public:
		StringCodecObj()
			: mStringObj(nullptr)
			, mConvType(CC_NULL)
		{
		}

		virtual ~StringCodecObj()
		{
			if (mStringObj)
				mStringObj->Release();
		}

		virtual runtimeObjectBase* doCall(doCallContext *context) override
		{
			do {
				if (context->GetParamCount() != 0)
					break;
				std::string rr;
				if (CC_MBCS2UNICODE == mConvType)
				{
					auto temp = notstd::ICONVext::mbcsToUnicode(*mStringObj->mVal);
					rr.append(reinterpret_cast<const char*>(temp.c_str()), temp.size() * 2);
				}
				else if (CC_UNICODE2MBCS == mConvType)
				{
					std::wstring temp(reinterpret_cast<const wchar_t*>(mStringObj->mVal->c_str()), mStringObj->mVal->size() / 2);
					rr = notstd::ICONVext::unicodeToMbcs(temp);
				}
				else if (CC_UTF82UNICODE == mConvType)
				{
					auto temp = notstd::ICONVext::utf8ToUnicode(*mStringObj->mVal);
					rr.append(reinterpret_cast<const char*>(temp.c_str()), temp.size() * 2);
				}
				else if (CC_UNICODE2UTF8 == mConvType)
				{
					std::wstring temp(reinterpret_cast<const wchar_t*>(mStringObj->mVal->c_str()), mStringObj->mVal->size() / 2);
					rr = notstd::ICONVext::unicodeToUtf8(temp);
				}
				else
					break;
				auto bo = new runtime::ObjectModule<BufferObject>;
				bo->mBuffer = rr;
				return bo;
			} while (0);
			return runtime::NullTypeObject::CreateNullTypeObject();
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
		else if (!strcmp(memName, "mbcs2unicode"))
		{
			auto sc = new ObjectModule<StringCodecObj>;
			sc->mStringObj = this;
			sc->mConvType = StringCodecObj::CC_MBCS2UNICODE;
			AddRef();
			return sc;
		}
		else if (!strcmp(memName, "unicode2mbcs"))
		{
			auto sc = new ObjectModule<StringCodecObj>;
			sc->mStringObj = this;
			sc->mConvType = StringCodecObj::CC_UNICODE2MBCS;
			AddRef();
			return sc;
		}
		else if (!strcmp(memName, "unicode2utf8"))
		{
			auto sc = new ObjectModule<StringCodecObj>;
			sc->mStringObj = this;
			sc->mConvType = StringCodecObj::CC_UNICODE2UTF8;
			AddRef();
			return sc;
		}
		else if (!strcmp(memName, "utf82unicode"))
		{
			auto sc = new ObjectModule<StringCodecObj>;
			sc->mStringObj = this;
			sc->mConvType = StringCodecObj::CC_UTF82UNICODE;
			AddRef();
			return sc;
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
