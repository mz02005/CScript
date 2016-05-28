#include "stdafx.h"
#include "cscript.h"
#include "CScriptEng.h"

extern "C"
{
	int InitializeCScriptEngine()
	{
		scriptAPI::SimpleCScriptEng::Init();
		return 0;
	}

	void UninitializeCScriptEngine()
	{
		scriptAPI::SimpleCScriptEng::Term();
	}

	int CreateCScriptCompile(CompilerHandle *handle)
	{
		*handle = new scriptAPI::ScriptCompiler;
		return 0;
	}

	int DestroyCScriptCompile(CompilerHandle handle)
	{
		delete reinterpret_cast<scriptAPI::ScriptCompiler*>(handle);
		return 0;
	}

	int PushCompileTimeName(CompilerHandle handle, const char *name)
	{
		return reinterpret_cast<scriptAPI::ScriptCompiler*>(handle)->PushName(name);
	}

	CompileResultHandle CompileCode(const char *filePathName, CompilerHandle compiler, int end)
	{
		scriptAPI::FileStream fs(filePathName);
		return reinterpret_cast<scriptAPI::ScriptCompiler*>(compiler)->Compile(&fs, !!end);
	}

	void ReleaseCompileResult(CompileResultHandle handle)
	{
		scriptAPI::ScriptCompiler::ReleaseCompileResult(handle);
	}

	int SaveConstStringTableToFile(CompileResultHandle crHandle, FILE *file)
	{
		return scriptAPI::ScriptCompiler::SaveConstStringTableInResultToFile(crHandle, file);
	}

	int LoadConstStringTableFromFile(CompileResultHandle crHandle, FILE *file)
	{
		return scriptAPI::ScriptCompiler::LoadConstStringTableToResultFromFile(crHandle, file);
	}

	int SaveCodeToFile(CompileResultHandle crHandle, FILE *file)
	{
		return scriptAPI::ScriptCompiler::SaveCodeToFile(crHandle, file);
	}

	int LoadCodeFromFile(CompileResultHandle crHandle, FILE *file)
	{
		return scriptAPI::ScriptCompiler::LoadCodeFromFile(crHandle, file);
	}

	int CreateVirtualMachine(VirtualMachineHandle *handle, 
		unsigned int stackSize, unsigned int stackFrameSize)
	{
		*handle = scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext(
			stackSize, stackFrameSize);
		return 0;
	}

	void DestroyVirtualMachine(VirtualMachineHandle handle)
	{
		scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(
			reinterpret_cast<scriptAPI::ScriptRuntimeContext*>(handle));
	}

	int PushRuntimeObject(VirtualMachineHandle handle, void *object)
	{
		return reinterpret_cast<scriptAPI::ScriptRuntimeContext*>(handle)->PushRuntimeObject(
			reinterpret_cast<runtime::runtimeObjectBase*>(object));
	}

	int VirtualMachineExecute(VirtualMachineHandle handle, CompileResultHandle compileResult)
	{
		return reinterpret_cast<scriptAPI::ScriptRuntimeContext*>(handle)->Execute(compileResult);
	}

	CodeHandle CopyAndClearCodeInCompileResult(CompileResultHandle resultHandle)
	{
		return scriptAPI::ScriptCompiler::CopyAndClearCompileResult(resultHandle);
	}

	int ReleaseCode(CodeHandle codeHandle)
	{
		scriptAPI::ScriptCompiler::ReleaseCompileCode(codeHandle);
		return 0;
	}

	int VirtualMachineExecuteCodeHandle(VirtualMachineHandle vmHandle, CodeHandle codeHandle, CompileResultHandle compileResult)
	{
		scriptAPI::ScriptRuntimeContext *rc = reinterpret_cast<scriptAPI::ScriptRuntimeContext*>(vmHandle);
		return rc->ExecuteCode(codeHandle, compileResult);
	}

	int ReplaceRuntimeFunc(const char *toReplace, void *runtimeObj, CompilerHandle cHandle, VirtualMachineHandle vmHandle)
	{
		scriptAPI::ScriptRuntimeContext *rc = reinterpret_cast<scriptAPI::ScriptRuntimeContext*>(vmHandle);
		return rc->ReplaceRuntimeFunc(toReplace, runtimeObj, cHandle);
	}

	int CreateInt32Instance(void **intObj)
	{
		*intObj = reinterpret_cast<void*>(new runtime::ObjectModule<runtime::intObject>);
		return 0;
	}

	int SetInt32Value(void *intObj, int value)
	{
		reinterpret_cast<runtime::intObject*>(intObj)->mVal = value;
		return 0;
	}

	int CreateFloatInstance(void **floatObj)
	{
		*floatObj = reinterpret_cast<void*>(new runtime::ObjectModule<runtime::floatObject>);
		return 0;
	}

	int SetFloatValue(void *floatObj, float value)
	{
		reinterpret_cast<runtime::floatObject*>(floatObj)->mVal = value;
		return 0;
	}

	int GetInt32Value(void *intObj, int *val)
	{
		runtime::runtimeObjectBase *o = reinterpret_cast<runtime::runtimeObjectBase*>(intObj);
		if (o->GetObjectTypeId() != runtime::DT_int32)
			return -1;
		*val = static_cast<runtime::intObject*>(o)->mVal;
		return 0;
	}

	int GetFloatValue(void *floatObj, float *val)
	{
		runtime::runtimeObjectBase *o = reinterpret_cast<runtime::runtimeObjectBase*>(floatObj);
		if (o->GetObjectTypeId() != runtime::DT_float)
			return -1;
		*val = static_cast<runtime::floatObject*>(o)->mVal;
		return 0;
	}
}
