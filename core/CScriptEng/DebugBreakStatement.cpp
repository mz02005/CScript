#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(DebugBreakStatement,Statement)

int DebugBreakStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	assert(false);
	mParentBlock = parent;
	return 0;
}

int DebugBreakStatement::GenerateInstruction(CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);
	gih.Insert_debug1_Instruction();
	return 0;
}
