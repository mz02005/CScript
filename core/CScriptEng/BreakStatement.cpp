#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(BreakStatement,Statement)

int BreakStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	mParentBlock = parent;
	return 0;
}

int BreakStatement::GenerateInstruction(CompileResult *compileResult)
{
	Statement *inLoop = isInLoopStatementBlock(SupportBreak);
	assert(inLoop);
	return inLoop->GenerateBreakStatementCode(this, compileResult);
}
