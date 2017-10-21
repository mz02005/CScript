#pragma once
#include "rtTypes.h"
#include "objType.h"
#include "arrayType.h"

namespace runtime {
	class runtimeContext;
}

namespace compiler {
	class SimpleCScriptEngContext;
}

namespace scriptAPI {
	class CSCRIPTENG_API ScriptSourceCodeStream
	{
	public:
		enum {
			Begin = SEEK_SET,
			End = SEEK_END,
			Current = SEEK_CUR,
		};
	public:
		virtual ~ScriptSourceCodeStream();
		// 得到开始的行数和列数
		virtual uint64_t GetBaseLine();
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
	
	class CSCRIPTENG_API SimpleCScriptEng
	{
	public:
		static void Init();
		static void Term();
	};

	struct ScriptLibReg
	{
		const char *objName;
		runtime::runtimeObjectBase *objInst;
	};

	struct LibEntry
	{
		std::string name;
		std::vector<ScriptLibReg> reg;
	};

	class CSCRIPTENG_API ScriptLibRegister
	{
	private:
		std::vector<LibEntry> *mLibEntryList;

	public:
		ScriptLibRegister();
		~ScriptLibRegister();
		int RegistLib(const std::string &libName, const ScriptLibReg *libReg, size_t c);
		std::vector<LibEntry>& GetLibList() { return *mLibEntryList; }
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
		ScriptLibRegister& GetLibRegister();
		HANDLE Compile(ScriptSourceCodeStream *stream, bool end = true);
		static HANDLE CreateCompileResult();
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
		int PushRuntimeObjectInLibs(ScriptLibRegister *reg);
		int Execute(HANDLE compileResult, bool recoveryStack = true, int *exitValue = nullptr);

		// 执行codeHandle指定的代码，但是使用compileResult指定的符号表
		int ExecuteCode(HANDLE code, HANDLE compileResult, bool recoveryStack = true, int *exitValue = nullptr);

		int ReplaceRuntimeFunc(const char *toReplace, void *runtimeObj, void *cHandle);
	};
}
