#include "stdafx.h"
#include "compile.h"

using namespace compiler;


IMPLEMENT_OBJINFO(ForStatement,Statement)

ForStatement::ForStatement()
	: mDeclExpression(nullptr)
	, mJudgementExpression(nullptr)
	, mIteratorExpression(nullptr)
{
}

ForStatement::~ForStatement()
{
	if (mDeclExpression)
		delete mDeclExpression;
	if (mJudgementExpression)
		delete mJudgementExpression;
	if (mIteratorExpression)
		delete mIteratorExpression;
}

int ForStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	int result;
	Symbol symbol;
	char c;

	mParentBlock = parent;

	if ((result = context->GetNextSymbol(symbol)) != 0)
		RETHELP(result);
	if (symbol.symbolOrig != "(")
		return -1;

	result = context->GetNextSymbol(symbol);
	if (result != 0)
		RETHELP(result);
	if (symbol.symbolOrig != ";")
	{
		context->GoBack();
		mDeclExpression = new PostfixExpression;
		if ((result = context->ParseExpressionEndWith(c, mDeclExpression, ";")) != 0)
			RETHELP(result);
	}

	result = context->GetNextSymbol(symbol);
	if (result != 0)
		RETHELP(result);
	if (symbol.symbolOrig != ";")
	{
		context->GoBack();
		mJudgementExpression = new PostfixExpression;
		if ((result = context->ParseExpressionEndWith(c, mJudgementExpression, ";")) != 0)
			RETHELP(result);
	}

	result = context->GetNextSymbol(symbol);
	if (result != 0)
		RETHELP(result);
	if (symbol.symbolOrig != ")")
	{
		context->GoBack();
		mIteratorExpression = new PostfixExpression;
		if ((result = context->ParseExpressionEndWith(c, mIteratorExpression, ")")) != 0)
			RETHELP(result);
	}

	if ((result = mStatementBlock.Compile(this, context, true)) < 0)
		return result;

#define EXPHELP(exp) \
	do { if (exp && (result = (exp)->CreateExpressionTree()) < 0) return result; } while (0)

	EXPHELP(mDeclExpression);
	EXPHELP(mJudgementExpression);
	EXPHELP(mIteratorExpression);

#undef EXPHELP

	return result;
}

int ForStatement::GenerateInstruction(CompileResult *compileResult)
{
	// 1 decl; 2 judgement; 3 iter
	// do 1
	// begin: do 2
	// jz end
	// {
	//	block
	// }
	// do 3
	// jmp begin
	// end:

	int r;
	GenerateInstructionHelper gih(compileResult);

	if (mDeclExpression) {
		if ((r = mDeclExpression->GenerateInstruction(this, compileResult)) < 0)
			return r;
	}

	uint32_t cycleBegin = compileResult->SaveCurrentCodePosition();

	if (mJudgementExpression) {
		if ((r = mJudgementExpression->GenerateInstruction(this, compileResult)) < 0)
			return r;
	}
	else
	{
		gih.Insert_createInt_Instruction(1);
	}

	uint32_t endPosToFill = gih.Insert_jz_Instruction(0);

	if ((r = mStatementBlock.GenerateInstruction(compileResult)) < 0)
		return r;

	uint32_t iterBegin = compileResult->SaveCurrentCodePosition();
	if (mIteratorExpression) {
		if ((r = mIteratorExpression->GenerateInstruction(this, compileResult)) < 0)
			return r;
		// 由于迭代语句会增加堆栈深度，且不会被抹掉，所以这里给消掉
		gih.Insert_pop_Instruction();
	}

	gih.Insert_jump_Instruction(cycleBegin);

	uint32_t endPos = compileResult->SaveCurrentCodePosition();
	gih.SetCode(endPosToFill, endPos);

	FillBreakList(compileResult, endPos);
	FillContinueList(compileResult, iterBegin);
	
	return 0;
}

int ForStatement::GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	uint32_t c = Statement::BlockDistance<ForStatement>(this, bs);
	// 需要退出栈帧
	for (uint32_t i = 0; i < c; i++)
		gih.Insert_popStackFrame_Instruction();

	// 跳转语句
	mBreakToFillList.push_back(gih.Insert_jump_Instruction(0));
	return 0;
}

int ForStatement::GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	uint32_t c = Statement::BlockDistance<ForStatement>(this, cs);
	// 需要退出栈帧
	for (uint32_t i = 0; i < c; i ++)
		gih.Insert_popStackFrame_Instruction();

	// 跳转语句
	mContinueToFillList.push_back(gih.Insert_jump_Instruction(0));
	return 0;
}
