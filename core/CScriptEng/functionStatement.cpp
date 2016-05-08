#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(FunctionStatement,Statement)

FunctionStatement::FunctionStatement(const std::string &name)
	: mName(name)
{
}

int FunctionStatement::ParseParamList(SimpleCScriptEngContext *context)
{
	int parseResult;
	Symbol symbol;

	for (;;)
	{
		int dataType;

		if ((parseResult = context->GetNextSymbol(symbol)) != 0)
			RETHELP(parseResult);
		if (symbol.type != symbol.Keywords
			|| !KeywordsTransTable::isDataType(symbol.keywordsType))
			return -1;
		dataType = symbol.keywordsType;
		// 形参，必须得有
		if ((parseResult = context->GetNextSymbol(symbol)) != 0)
			RETHELP(parseResult);
		// 形参必须是符号类型
		if (symbol.type != Symbol::CommonSymbol)
			return -1;

		Param param;
		param.type = dataType;
		param.name = symbol.symbolOrig;
		mParamList.emplace_back(param);

		if ((parseResult = context->GetNextSymbol(symbol)) != 0)
			RETHELP(parseResult);
		if (symbol.symbolOrig == ")")
			break;
		if (symbol.symbolOrig != ",")
			return -1;
	}
	return 0;
}

int FunctionStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	int parseResult;
	Symbol symbol;

	if ((parseResult = context->GetNextSymbol(symbol)) != 0)
		RETHELP(parseResult);
	if (symbol.symbolOrig != "(")
		return -1;

	if ((parseResult = ParseParamList(context)) != 0)
		RETHELP(parseResult);

	if ((parseResult = context->GetNextSymbol(symbol)) != 0)
		RETHELP(parseResult);
	if (symbol.symbolOrig != "{")
		return -1;

	if ((parseResult = mFunctionBlock.Compile(parent, context, false)) != 0)
		RETHELP(parseResult);

	return 0;
}

int FunctionStatement::GenerateInstruction(CompileResult *compileResult)
{
	return 0;
}
