#include "stdafx.h"
#include "compile.h"

using namespace compiler;


IMPLEMENT_OBJINFO(WhileStatement,Statement)

int WhileStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	char c;
	Symbol symbol;

	mParentBlock = parent;

	int parseResult = context->GetNextSymbol(symbol);
	if (parseResult != 0)
		RETHELP(parseResult);
	if (symbol.symbolOrig != "(")
		return -1;

	if ((parseResult = context->ParseExpressionEndWith(c, &mJudgementExpression, ")")) != 0)
		RETHELP(parseResult);
	
	bool hasBrace = false;
	if ((parseResult = context->GetNextSymbol(symbol)) != 0)
		return -1;
	if (symbol.symbolOrig == "{")
		hasBrace = true;
	else
		context->GoBack();
	if ((parseResult = mStatementBlock.Compile(this, context, hasBrace)) != 0)
		RETHELP(parseResult);

	return mJudgementExpression.CreateExpressionTree();
}

int WhileStatement::GenerateInstruction(CompileResult *compileResult)
{
	int r;
	uint32_t beginWhilePos, endWhileToWrite;
	GenerateInstructionHelper gih(compileResult);

	runtime::CommonInstruction ci = 0;
	runtime::Instruction *inst = reinterpret_cast<runtime::Instruction*>(&ci);
	
	beginWhilePos = compileResult->SaveCurrentCodePosition();

	if ((r = mJudgementExpression.CreateExpressionTree()) < 0)
		return r;
	if ((r = mJudgementExpression.GenerateInstruction(this, compileResult)) < 0)
		return r;

	endWhileToWrite = gih.Insert_jz_Instruction(0);

	if ((r = mStatementBlock.GenerateInstruction(compileResult)) < 0)
		return r;

	gih.Insert_jump_Instruction(beginWhilePos);

	uint32_t endPos = compileResult->SaveCurrentCodePosition();
	gih.SetCode(endWhileToWrite, endPos);

	FillBreakList(compileResult, endPos);
	FillContinueList(compileResult, beginWhilePos);

	return r;
}

int WhileStatement::GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	// 需要退出栈帧
	uint32_t c;
	bool b = Statement::BlockDistance<WhileStatement>(this, bs, c);
	if (b)
	{
		for (uint32_t x = 0; x < c; x++)
			gih.Insert_leaveBlock_Instruction();
	}

	// 跳转语句
	mBreakToFillList.push_back(gih.Insert_jump_Instruction(0));

	return 0;
}

int WhileStatement::GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	// 需要退出栈帧
	uint32_t c;
	bool b = Statement::BlockDistance<WhileStatement>(this, cs, c);
	if (b)
	{
		for (uint32_t x = 0; x < c; x++)
			gih.Insert_leaveBlock_Instruction();
	}

	// 跳转语句
	mContinueToFillList.push_back(gih.Insert_jump_Instruction(0));

	return 0;
}
