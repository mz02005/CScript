#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(ReturnStatement,Statement)

int ReturnStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	if (parent == nullptr
		|| parent->GetThisObjInfo() != OBJECT_INFO(FunctionStatement))
		return -1;

	return 0;
}

int ReturnStatement::GenerateInstruction(CompileResult *compileResult)
{
	return 0;
}
