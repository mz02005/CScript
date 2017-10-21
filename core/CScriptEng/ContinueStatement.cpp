#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(ContinueStatement,Statement)

int ContinueStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	assert(false);
	mParentBlock = parent;
	return 0;
}

int ContinueStatement::GenerateInstruction(CompileResult *compileResult)
{
	Statement *inLoop = isInLoopStatementBlock(SupportContinue);
	assert(inLoop);
	return inLoop->GenerateContinueStatementCode(this, compileResult);
}
