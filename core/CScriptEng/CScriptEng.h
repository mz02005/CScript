#pragma once
#include "config.h"
#include "notstd/notstd.h"
#include "cscriptBase.h"

#define SCRIPT_TRACE scriptLog::LogTool()
#define SCRIPT_TRACE_(flags) scriptLog::LogTool((uint32_t)flags)

namespace scriptLog {
	class LogTool
	{
	public:
		enum {
			TraceGeneral = 0x000001,
			TraceCom = 0x000002,
			TraceQI = 0x000004,
			TraceRegistrar = 0x000008,
			TraceRefcount = 0x000010,
			TraceWindowing = 0x000020,
			TraceControls = 0x000040,
			TraceHosting = 0x000080,
			TraceDBClient = 0x000100,
			TraceDBProvider = 0x000200,
			TraceSnapin = 0x000400,
			TraceNotImpl = 0x000800,
			TraceAllocation = 0x001000,
			TraceException = 0x002000,
			TraceTime = 0x004000,
			TraceCache = 0x008000,
			TraceStencil = 0x010000,
			TraceString = 0x020000,
			TraceMap = 0x040000,
			TraceUtil = 0x080000,
			TraceSecurity = 0x100000,
			TraceSync = 0x200000,
			TraceISAPI = 0x400000,
			TraceUser = 0x80000,

			TraceAll = (uint32_t)-1,
		};

	private:
		// 需要显示哪些类型的信息
		static uint32_t mLogFlags;

		uint32_t mMyFlags;
		bool mToTrace;

	public:
		static void SetLogFlags(uint32_t newFlags);
		static uint32_t GetLogFlags() { return mLogFlags; }
		LogTool(uint32_t theFlags = 0);
		void operator()(const char *format, ...);
	};
}

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

		virtual bool isGreaterThan(const runtimeObjectBase *obj);
		virtual bool isEqual(const runtimeObjectBase *obj);
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
		virtual bool isGreaterThan(const runtimeObjectBase *obj);
		virtual bool isEqual(const runtimeObjectBase *obj);

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
		virtual bool isGreaterThan(const runtimeObjectBase *obj);
		virtual bool isEqual(const runtimeObjectBase *obj);

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
		virtual bool isGreaterThan(const runtimeObjectBase *obj);
		virtual bool isEqual(const runtimeObjectBase *obj);

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

		virtual bool isGreaterThan(const runtimeObjectBase *obj);
		virtual bool isEqual(const runtimeObjectBase *obj);

		virtual stringObject* toString();
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
		typedef T::InnerDataType ReturnType;
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

namespace runtime {
	class runtimeContext;
}

namespace compiler {
	class SimpleCScriptEngContext;
}

namespace scriptAPI {
	class ScriptSourceCodeStream
	{
	public:
		enum {
			Begin = SEEK_SET,
			End = SEEK_END,
			Current = SEEK_CUR,
		};
	public:
		virtual void Close() = 0;
		virtual int Flush() = 0;
		virtual int Read(uint8_t *data, int offset, int count) = 0;
		virtual int64_t Seek(int64_t offset, int origPosition) = 0;
		virtual int SetLength(int64_t length) = 0;
		virtual int Write(const uint8_t *data, int offset, int count) = 0;
	};

	class CSCRIPTENG_API FileStream : public ScriptSourceCodeStream
	{
	private:
		FILE *mFile;

	public:
		FileStream(const char *filePathName);
		FileStream();
		virtual ~FileStream();

		int Open(const char *filePathName);

		virtual void Close();
		virtual int Flush();
		virtual int Read(uint8_t *data, int offset, int count);
		virtual int64_t Seek(int64_t offset, int origPosition);
		virtual int SetLength(int64_t length);
		virtual int Write(const uint8_t *data, int offset, int count);
	};
	
	class CSCRIPTENG_API SimpleCScriptEng
	{
	public:
		static void Init();
		static void Term();
	};

	class CSCRIPTENG_API StringStream : public ScriptSourceCodeStream
	{
	private:
		size_t mSize;
		char *mBuf;
		char *mCur;

	public:
		StringStream(const char *str, size_t size);
		virtual ~StringStream();

		virtual void Close();
		virtual int Flush();
		virtual int Read(uint8_t *data, int offset, int count);
		virtual int64_t Seek(int64_t offset, int origPosition);
		virtual int SetLength(int64_t length);
		virtual int Write(const uint8_t *data, int offset, int count);
	};
	
	class CSCRIPTENG_API ScriptCompiler
	{
	private:
		compiler::SimpleCScriptEngContext *mCompilerContext;

	public:
		struct CompileCode
		{
			uint32_t sizeInUint32;
			uint32_t *code;
		};

	public:
		ScriptCompiler();
		~ScriptCompiler();
		int PushName(const char *name);
		int FindGlobalName(const char *name);
		HANDLE Compile(ScriptSourceCodeStream *stream, bool end = true);
		static int SaveConstStringTableInResultToFile(HANDLE crHandle, FILE *file);
		static int LoadConstStringTableToResultFromFile(HANDLE crHandle, FILE *file);
		static int SaveCodeToFile(HANDLE crHandle, FILE *file);
		static int LoadCodeFromFile(HANDLE crHandle, FILE *file);
		// 返回代表代码区域的句柄
		static HANDLE CopyAndClearCompileResult(HANDLE compileResult);
		static void ReleaseCompileResult(HANDLE result);
		static void ReleaseCompileCode(HANDLE code);
	};

	class CSCRIPTENG_API ScriptRuntimeContext
	{
	private:
		runtime::runtimeContext *mContextInner;
		ScriptRuntimeContext(uint32_t stackSize, uint32_t stackFrameSize);

	public:
		static ScriptRuntimeContext* CreateScriptRuntimeContext(
			uint32_t stackSize = 512, uint32_t stackFrameSize = 128);
		static void DestroyScriptRuntimeContext(ScriptRuntimeContext *context);
		~ScriptRuntimeContext();
		int PushRuntimeObject(runtime::runtimeObjectBase *obj);
		int Execute(HANDLE compileResult);

		// 执行codeHandle指定的代码，但是使用compileResult指定的符号表
		int ExecuteCode(HANDLE code, HANDLE compileResult);

		int ReplaceRuntimeFunc(const char *toReplace, void *runtimeObj, void *cHandle);
	};
}
