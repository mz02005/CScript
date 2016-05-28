#pragma once
#include <stdio.h>

#ifdef IMPORT_APIS
#define CSCRIPT_API __declspec(dllimport)
#pragma comment(lib, "CScriptEng.lib")
#else
#define CSCRIPT_API
#endif

extern "C" {

typedef void* CompilerHandle;
typedef void* CompileResultHandle;
typedef void* VirtualMachineHandle;
typedef void* CodeHandle;

CSCRIPT_API int InitializeCScriptEngine();
CSCRIPT_API void UninitializeCScriptEngine();

CSCRIPT_API int CreateCScriptCompile(CompilerHandle *handle);
CSCRIPT_API int DestroyCScriptCompile(CompilerHandle handle);
CSCRIPT_API int PushCompileTimeName(CompilerHandle handle, const char *name);
CSCRIPT_API CompileResultHandle CompileCode(const char *filePathName, CompilerHandle compiler, int end);
CSCRIPT_API void ReleaseCompileResult(CompileResultHandle handle);
CSCRIPT_API int SaveConstStringTableToFile(CompileResultHandle crHandle, FILE *file);
CSCRIPT_API int LoadConstStringTableFromFile(CompileResultHandle crHandle, FILE *file);
CSCRIPT_API int SaveCodeToFile(CompileResultHandle crHandle, FILE *file);
CSCRIPT_API int LoadCodeFromFile(CompileResultHandle crHandle, FILE *file);

CSCRIPT_API int CreateVirtualMachine(VirtualMachineHandle *handle, 
	unsigned int stackSize, unsigned int stackFrameSize);
CSCRIPT_API void DestroyVirtualMachine(VirtualMachineHandle handle);
CSCRIPT_API int PushRuntimeObject(VirtualMachineHandle handle, void *object);
CSCRIPT_API int VirtualMachineExecute(VirtualMachineHandle handle, CompileResultHandle compileResult);
CSCRIPT_API int ReplaceRuntimeFunc(const char *toReplace, void *runtimeObj, CompilerHandle cHandle, VirtualMachineHandle vmHandle);

CSCRIPT_API int CreateInt32Instance(void **intObj);
CSCRIPT_API int SetInt32Value(void *intObj, int value);
CSCRIPT_API int CreateFloatInstance(void **floatObj);
CSCRIPT_API int SetFloatValue(void *floatObj, float value);

CSCRIPT_API int GetInt32Value(void *intObj, int *val);
CSCRIPT_API int GetFloatValue(void *floatObj, float *val);

CSCRIPT_API CodeHandle CopyAndClearCodeInCompileResult(CompileResultHandle resultHandle);
CSCRIPT_API int ReleaseCode(CodeHandle codeHandle);
CSCRIPT_API int VirtualMachineExecuteCodeHandle(VirtualMachineHandle vmHandle, CodeHandle codeHandle, CompileResultHandle compileResult);

}
