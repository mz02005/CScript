#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(ReturnStatement,Statement)

int ReturnStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	if (parent == NULL)
		return -1;

	Statement *p = parent;
	while (p && !p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
	{
		p = p->GetParent();
	}
	if (!p)
	{
		SCRIPT_TRACE("return statement not in function object.\n");
		return -1;
	}

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

	gih.Insert_saveToA_Instruction();

	Statement *p = GetParent();
	while (p && !p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		p = p->GetParent();
	if (!p)
		return -1;
	//gih.Insert_popStackFrameAndSaveResult_Instruction(
	//	Statement::BlockDistance<FunctionStatement>(static_cast<FunctionStatement*>(p), this));

	uint32_t c;
	bool b = Statement::BlockDistance<FunctionStatement>(
		static_cast<FunctionStatement*>(p), this, c);
	for (uint32_t x = 0; x < c; x++)
		gih.Insert_leaveBlock_Instruction();
	gih.Insert_return_Instruction();
	return r;
}
