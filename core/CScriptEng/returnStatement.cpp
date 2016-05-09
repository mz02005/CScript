#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(ReturnStatement,Statement)

int ReturnStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	if (parent == nullptr
		|| parent->GetThisObjInfo() != OBJECT_INFO(FunctionStatement))
		return -1;

	char c;
	int parseResult;
	if ((parseResult = context->ParseExpressionEndWith(c, &mExp, ";")) < 0)
		return parseResult;

	return parseResult;
}

int ReturnStatement::GenerateInstruction(CompileResult *compileResult)
{
	// 从函数中的pushStackFrame返回
	GenerateInstructionHelper gih(compileResult);
	int r = mExp.GenerateInstruction(this, compileResult);
	if (r < 0)
		return r;
	gih.Insert_return_Instruction();
	return r;
}
