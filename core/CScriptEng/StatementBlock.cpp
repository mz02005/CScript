#include "stdafx.h"
#include "compile.h"
#include <algorithm>

using namespace compiler;

IMPLEMENT_OBJINFO(StatementBlock,Statement)

StatementBlock::StatementBlock()
: mSaveFrame(true)
{
}

StatementBlock::~StatementBlock()
{
	for (auto iter = mStatementList.begin(); iter != mStatementList.end(); iter++)
		delete (*iter);
	mStatementList.clear();
}

//runtime::CommonInstruction StatementBlock::DeclTypeToCreateInstuction(uint32_t declType) const
//{
//	assert(KeywordsTransTable::isDataType(declType));
//	return runtime::VM_createchar + (declType - compiler::KeywordsTransTable::CK_CHAR);
//}

void StatementBlock::InsertCreateTypeInstructionByDeclType(
	uint32_t declType, GenerateInstructionHelper *giHelper)
{
	//assert(KeywordsTransTable::isDataType(declType));
	switch (declType)
	{
	case KeywordsTransTable::CK_CHAR:
		giHelper->Insert_createChar_Instruction(0);
		break;

	case KeywordsTransTable::CK_SHORT:
		giHelper->Insert_createShort_Instruction(0);
		break;

	case KeywordsTransTable::CK_USHORT:
		giHelper->Insert_createUshort_Instruction(0);
		break;

	case KeywordsTransTable::CK_INT:
		giHelper->Insert_createInt_Instruction(0);
		break;

	case KeywordsTransTable::CK_UINT:
		giHelper->Insert_createUint_Instruction(0);
		break;

	case KeywordsTransTable::CK_FLOAT:
		giHelper->Insert_createFloat_Instruction(0.f);
		break;

	case KeywordsTransTable::CK_DOUBLE:
		giHelper->Insert_createDouble_Instruction(0.f);
		break;

	case KeywordsTransTable::CK_STRING:
		giHelper->Insert_createString_Instruction("");
		break;

	case KeywordsTransTable::CK_ARRAY:
		giHelper->Insert_createArray_Instruction();
		break;

	default:
		SCRIPT_TRACE("Invalid data type to create\n");
		exit(1);
	}
}

int StatementBlock::GenerateInstruction(CompileResult *compileResult)
{
	int r = 0;

	GenerateInstructionHelper gih(compileResult);

	// 维护栈帧
	if (mParentBlock && mSaveFrame)
		gih.Insert_pushStackFrame_Instruction();

	// 为声明的类型生成将基本数据类型压入堆栈的操作
	for (auto iter = mLocalNameType.begin(); iter != mLocalNameType.end(); iter++)
	{
		if (*iter != 0)
		{
			InsertCreateTypeInstructionByDeclType(*iter, &gih);
		}
	}

	for (auto iter = mStatementList.begin(); iter != mStatementList.end(); iter++)
	{
		if ((r = (*iter)->GenerateInstruction(compileResult)) < 0)
			return r;
	}

	if (mParentBlock && mSaveFrame)
		gih.Insert_popStackFrame_Instruction();

	return r;
}

int StatementBlock::PushName(const char *name, uint32_t declType)
{
	// 名字重复了
	if (mLocalNameStack.find(std::string(name)) != mLocalNameStack.end())
	{
		SCRIPT_TRACE("The symbol or variable (%s) is defined.\n", name);
		return -11;
	}
	mLocalNameStack[std::string(name)] = mLocalNameType.size();
	mLocalNameType.push_back(declType);
	return 0;
}

bool StatementBlock::FindName(const char *name, uint32_t &level, uint32_t &index) const
{
	const StatementBlock *p = this;
	level = 0;
	while (p)
	{
		auto iter = p->mLocalNameStack.find(name);
		if (iter != p->mLocalNameStack.end())
		{
			index = iter->second;
			return true;
		}
		else
		{
			if (p->GetParent()) {
				if (p->mSaveFrame)
					level ++;
				p = p->GetParent()->GetBlockParent();
			}
			else
				return false;
		}
	}
	return false;
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

int StatementBlock::Compile(Statement *parent, SimpleCScriptEngContext *context, bool mayLackOfBrace)
{
	int r;
	bool hasBraceAtBegin = false;
	Symbol symbol;

	mParentBlock = parent;

	if (context->GetNextSymbol(symbol) != 0)
		return -1;
	if (!mayLackOfBrace && symbol.symbolOrig != "{")
		return -1;
	if (symbol.symbolOrig == "{")
	{
		// 
		if (!mParentBlock)
		{
			// 为了处理{符号出现在代码第一个符号的情况
			context->GoBack();
		}
		else
			hasBraceAtBegin = true;
	}
	else {
		if (mayLackOfBrace)
			context->GoBack();
	}

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
				if ((loopStatement = isInLoopStatementBlock(Statement::SupportBreak)) == nullptr)
					return -1;
				if (context->GetNextSymbol(symbol) != 0
					|| symbol.symbolOrig != ";")
					return -1;
				BreakStatement *breakStatement = new BreakStatement;
				AddStatement(breakStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_CONTINUE)
			{
				Statement *loopStatement;
				if ((loopStatement = isInLoopStatementBlock(Statement::SupportContinue)) == nullptr)
					return -1;
				if (context->GetNextSymbol(symbol) != 0
					|| symbol.symbolOrig != ";")
					return -1;
				ContinueStatement *conStatement = new ContinueStatement;
				AddStatement(conStatement);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_FUNCTION)
			{
				if ((parseResult = context->GetNextSymbol(symbol)) != 0)
					return -1;
				// 名字必须是普通的标志符
				if (symbol.type != Symbol::CommonSymbol)
					return -1;
				FunctionStatement *fs = new FunctionStatement(symbol.symbolOrig);
				if ((parseResult = fs->Compile(this, context)) != 0)
				{
					delete fs;
					RETHELP(parseResult);
				}
				AddStatement(fs);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_RETURN)
			{
				if ((parseResult = context->GetNextSymbol(symbol)) != 0)
					return -1;
				if (symbol.symbolOrig != ";")
					return -1;
				ReturnStatement *rs = new ReturnStatement;
				if ((parseResult = rs->Compile(this, context)) < 0)
					return parseResult;
				AddStatement(rs);
			}
			else if (symbol.keywordsType == KeywordsTransTable::CK_DEBUGBREAK)
			{
				if (context->GetNextSymbol(symbol) != 0
					|| symbol.symbolOrig != ";")
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
			if (mayLackOfBrace && !hasBraceAtBegin && mParentBlock)
			{
				break;
			}
			if ((parseResult = context->GetNextSymbol(symbol)) == 0)
			{
				if (symbol.symbolOrig == ";")
				{
					if (!hasBraceAtBegin)
						return 0;
				}
				context->GoBack();
			}
		}
		else if (symbol.type == Symbol::terminalChar)
		{
			if (symbol.symbolOrig == "}")
			{
				if (!hasBraceAtBegin)
				{
					// 出现了右花括号，而当前块不是由花括号开始的，那就表明当前块结束了
					if (mParentBlock)
					{
						context->GoBack();
						return 0;
					}
					return -1;
				}
				return 0;
			}
			else if (symbol.symbolOrig == ";")
			{
				// 不能因为一个分号而退出了顶层的编译
				if (!hasBraceAtBegin && mParentBlock != nullptr)
					return 0;
			}
			else if (symbol.symbolOrig == "{")
			{
				// 一个空的block
				context->GoBack();
				StatementBlock *blockStatement = new StatementBlock;
				int result = blockStatement->Compile(this, context);
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
			if (!hasBraceAtBegin && mParentBlock != nullptr)
				return 0;
		}
	}

	return r;
}
