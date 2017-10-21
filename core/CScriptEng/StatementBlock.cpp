#include "stdafx.h"
#include "compile.h"
#include <algorithm>

using namespace compiler;

IMPLEMENT_OBJINFO(StatementBlock,Statement)

StatementBlock::StatementBlock()
{
}

StatementBlock::~StatementBlock()
{
	for (auto iter = mStatementList.begin(); iter != mStatementList.end(); iter++)
		delete (*iter);
	mStatementList.clear();
}

int StatementBlock::GenerateInstruction(CompileResult *compileResult)
{
	int r = 0;

	GenerateInstructionHelper gih(compileResult);

	gih.Insert_enterBlock_Instruction();

	for (auto iter = mStatementList.begin(); iter != mStatementList.end(); iter++)
	{
		if ((r = (*iter)->GenerateInstruction(compileResult)) < 0)
			return r;
	}

	// 如果父亲是函数，则在末尾增加一个return语句
	// 这可以保证如果用户忘了写return语句，堆栈还能够平衡
	if (mParentBlock->GetThisObjInfo() == OBJECT_INFO(FunctionStatement)/*
		&& !static_cast<FunctionStatement*>(mParentBlock)->isTopLevelFun()*/)
	{
		// 返回的是整数0
		gih.Insert_createInt_Instruction(0);
		gih.Insert_saveToA_Instruction();

		gih.Insert_leaveBlock_Instruction();
		gih.Insert_return_Instruction();
	}
	
	gih.Insert_leaveBlock_Instruction();

	return r;
}

bool StatementBlock::RegistNameInBlock(const char *name, uint32_t declType)
{
	// 名字重复了
	if (mLocalNameStack.find(std::string(name)) != mLocalNameStack.end())
	{
		SCRIPT_TRACE("The symbol or variable (%s) is defined.\n", name);
		return false;
	}

	// TODO: 下面这部分代码可以整理一下
	FunctionStatement *fs = GetFunctionParent();
	assert(fs);
	uint32_t index = fs->ReservedVariableRoom(declType);
	mLocalNameStack[name] = mPointerToType.size();
	mPointerToType.push_back(index);
	return true;
}

bool StatementBlock::FindNameInBlock(const char *name, uint32_t &l, uint32_t &i) const
{
	auto iter = mLocalNameStack.find(name);
	if (iter != mLocalNameStack.end())
	{
		i = mPointerToType[iter->second];
		return true;
	}
	return GetParent()->FindName(name, l, i);
}

void StatementBlock::AddStatement(Statement *statement)
{
	statement->mParentBlock = this;
	mStatementList.push_back(statement);
}

int StatementBlock::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	// 这种调用总是意味着读到了左花括号
	return Compile(parent, context, true);
}

int StatementBlock::Compile(Statement *parent, SimpleCScriptEngContext *context, bool beginWithBrace)
{
	int r;
	Symbol symbol;

	mParentBlock = parent;

	if (context->GetNextSymbol(symbol) != 0)
		return -1;
	if (symbol.symbolOrig == "}" && !beginWithBrace)
		return -2;
	context->GoBack();

	int parseResult;
	while ((r = context->GetNextSymbol(symbol)) == 0)
	{
		if (symbol.type == Symbol::Keywords)
		{
			if (symbol.keywordsType == KeywordsTransTable::CK_IF)
			{
				IfConditionStatement *ifStatement = new IfConditionStatement;
				if ((parseResult = ifStatement->Compile(this, context)) != 0)
				{
					delete ifStatement;
					RETHELP(parseResult);
				}
				AddStatement(ifStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_FOR)
			{
				ForStatement *forStatement = new ForStatement;
				if ((parseResult = forStatement->Compile(this, context)) != 0)
				{
					delete forStatement;
					RETHELP(parseResult);
				}
				AddStatement(forStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_DO)
			{
				DoWhileStatement *doWhileStatement = new DoWhileStatement;
				if ((parseResult = doWhileStatement->Compile(this, context)) != 0)
				{
					delete doWhileStatement;
					RETHELP(parseResult);
				}
				AddStatement(doWhileStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_WHILE)
			{
				WhileStatement *whileStatement = new WhileStatement;
				if ((parseResult = whileStatement->Compile(this, context)) != 0)
				{
					delete whileStatement;
					RETHELP(parseResult);
				}
				AddStatement(whileStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_SWITCH)
			{
				SwitchStatement *switchStatement = new SwitchStatement;
				if ((parseResult = switchStatement->Compile(this, context)) != 0)
				{
					delete switchStatement;
					RETHELP(parseResult);
				}
				AddStatement(switchStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_BREAK)
			{
				Statement *loopStatement;
				// 没有循环可以跳出
				if ((loopStatement = isInLoopStatementBlock(Statement::SupportBreak)) == NULL)
					return -1;
				if (!context->GetNextSymbolMustBe(symbol, ";"))
					return -1;
				BreakStatement *breakStatement = new BreakStatement;
				AddStatement(breakStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_CONTINUE)
			{
				Statement *loopStatement;
				if ((loopStatement = isInLoopStatementBlock(Statement::SupportContinue)) == NULL)
					return -1;
				if (!context->GetNextSymbolMustBe(symbol, ";"))
					return -1;
				ContinueStatement *conStatement = new ContinueStatement;
				AddStatement(conStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_FUNCTION)
			{
				if ((parseResult = context->GetNextSymbol(symbol)) != 0)
					return -1;
				
				if (symbol.type != Symbol::CommonSymbol)
				{
					// 如果没有名字，则说明是进入了function表达式模式
					context->GoBack(); context->GoBack();
					PureExpressionStatement *pureExp = new PureExpressionStatement;
					if ((parseResult = pureExp->Compile(this, context)) != 0)
					{
						delete pureExp;
						RETHELP(parseResult);
					}
					AddStatement(pureExp);
				}
				else {
					uint32_t l = 0, i = 0;
					// 名字重复
					if (FindName(symbol.symbolOrig.c_str(), l, i)
						&& CheckValidLayer(l - 1))
					{
						SCRIPT_TRACE("name [%s] already exists.", symbol.symbolOrig.c_str());
						return -1;
					}
					FunctionStatement *fs = new FunctionStatement(symbol.symbolOrig);
					if ((parseResult = fs->Compile(this, context)) != 0)
					{
						delete fs;
						RETHELP(parseResult);
					}
					AddStatement(fs);
				}
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_RETURN)
			{
				ReturnStatement *rs = new ReturnStatement;
				if ((parseResult = rs->Compile(this, context)) < 0)
					return parseResult;
				AddStatement(rs);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_DEBUGBREAK)
			{
				if (!context->GetNextSymbolMustBe(symbol, ";"))
					return -1;
				DebugBreakStatement *db = new DebugBreakStatement;
				AddStatement(db);
			}
			else if (KeywordsTransTable::isDataType(symbol.keywordsType))
			{
				DeclareStatement *declStatement = new DeclareStatement;
				declStatement->mTypeString = symbol.symbolOrig;
				declStatement->mDeclType = symbol.keywordsType;
				if ((parseResult = declStatement->Compile(this, context)) != 0)
				{
					delete declStatement;
					RETHELP(parseResult);
				}
				AddStatement(declStatement);
			}
			else
			{
				return -1;
			}
		}
		else if (symbol.type == Symbol::terminalChar)
		{
			if (symbol.symbolOrig == "}")
			{
				// 顶层function是不会遇到}的
				if (GetParent()->isInheritFrom(OBJECT_INFO(FunctionStatement))
					&& GetFunctionParent()->isTopLevelFun())
					return -1;
				if (!beginWithBrace)
					return -1;
				return 0;
			}
			else if (symbol.symbolOrig == ";")
			{
			}
			else if (symbol.symbolOrig == "{")
			{
				// 一个空的block
				StatementBlock *blockStatement = new StatementBlock;
				int result = blockStatement->Compile(this, context, true);
				if (result != 0) {
					delete blockStatement;
					RETHELP(result);
				}
				AddStatement(blockStatement);
			}
			else
			{
				// 如果语句开头如下，则匹配到下面的代码了
				// *p = asdfsdf;
				context->GoBack();
				PureExpressionStatement *expStatement = new PureExpressionStatement;
				if ((parseResult = expStatement->Compile(this, context)) != 0)
					RETHELP(parseResult);
				AddStatement(expStatement);
			}
		}
		else
		{
			context->GoBack();
			PureExpressionStatement *pureExp = new PureExpressionStatement;
			if ((parseResult = pureExp->Compile(this, context)) != 0)
			{
				delete pureExp;
				RETHELP(parseResult);
			}
			AddStatement(pureExp);
		}

		if (!beginWithBrace)
			return 0;
	}

	return r;
}
