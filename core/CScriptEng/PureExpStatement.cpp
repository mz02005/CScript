#include "stdafx.h"
#include "compile.h"

using namespace compiler;


IMPLEMENT_OBJINFO(PureExpressionStatement,Statement)

int PureExpressionStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	char c;
	mParentBlock = parent;
	int result = context->ParseExpressionEndWith(c, &mExp, ";");
	if (result != 0)
		RETHELP(result);
	return mExp.CreateExpressionTree();
}

int PureExpressionStatement::GenerateInstruction(CompileResult *compileResult)
{
	return mExp.GenerateInstruction(this, compileResult);
}
