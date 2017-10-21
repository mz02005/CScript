#include "stdafx.h"
#include "compile.h"

using namespace compiler;

int BreakContinueSupport::FillBreakList(CompileResult *compileResult, uint32_t actureJumpPos)
{
	GenerateInstructionHelper gih(compileResult);
	for (auto iter = mBreakToFillList.begin(); iter != mBreakToFillList.end(); iter++)
	{
		gih.SetCode(*iter, actureJumpPos);
	}
	return 0;
}

int BreakContinueSupport::FillContinueList(CompileResult *compileResult, uint32_t actureJumpPos)
{
	GenerateInstructionHelper gih(compileResult);
	for (auto iter = mContinueToFillList.begin(); iter != mContinueToFillList.end(); iter++)
	{
		gih.SetCode(*iter, actureJumpPos);
	}
	return 0;
}
