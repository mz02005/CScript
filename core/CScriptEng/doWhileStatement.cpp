#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(DoWhileStatement,Statement)

int DoWhileStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	mParentBlock = parent;

	int parseResult = mStatementBlock.Compile(this, context, false);
	if (parseResult != 0)
		RETHELP(parseResult);

	Symbol symbol;
	parseResult = context->GetNextSymbol(symbol);
	if (parseResult != 0)
		RETHELP(parseResult);
	if (symbol.symbolOrig != "while")
		return -1;
	parseResult = context->GetNextSymbol(symbol);
	if (parseResult != 0)
		RETHELP(parseResult);
	if (symbol.symbolOrig != "(")
		return -1;

	char c;
	parseResult = context->ParseExpressionEndWith(c, &mJudgementExpression, ")");
	if (parseResult != 0)
		RETHELP(parseResult);

	parseResult = context->GetNextSymbol(symbol);
	if (parseResult != 0)
		RETHELP(parseResult);
	if (symbol.symbolOrig != ";")
		return -1;

	return mJudgementExpression.CreateExpressionTree();
}

int DoWhileStatement::GenerateInstruction(CompileResult *compileResult)
{
	int r;
	GenerateInstructionHelper gih(compileResult);

	uint32_t beginPos = compileResult->SaveCurrentCodePosition();
	if ((r = mStatementBlock.GenerateInstruction(compileResult)) < 0)
		return r;

	if ((r = mJudgementExpression.GenerateInstruction(this, compileResult)) < 0)
		return r;

	uint32_t endPosToFill = gih.Insert_jz_Instruction(0);

	gih.Insert_jump_Instruction(beginPos);

	uint32_t endPos = compileResult->SaveCurrentCodePosition();

	gih.SetCode(endPosToFill, endPos);

	FillBreakList(compileResult, endPos);
	FillContinueList(compileResult, beginPos);

	return 0;
}

int DoWhileStatement::GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	uint32_t c = Statement::BlockDistance<DoWhileStatement>(this, bs);
	// 需要退出栈帧
	for (uint32_t i = 0; i < c; i++)
		gih.Insert_popStackFrame_Instruction();

	// 跳转语句
	mBreakToFillList.push_back(gih.Insert_jump_Instruction(0));
	return 0;
}

int DoWhileStatement::GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	uint32_t c = Statement::BlockDistance<DoWhileStatement>(this, cs);
	// 需要退出栈帧
	for (uint32_t i = 0; i < c; i ++)
		gih.Insert_popStackFrame_Instruction();

	// 跳转语句
	mContinueToFillList.push_back(gih.Insert_jump_Instruction(0));
	return 0;
}
