#include "stdafx.h"
#include "compile.h"

using namespace compiler;

const char FunctionStatement::mEntryFuncName[] =
	"__entry__";

IMPLEMENT_OBJINFO(FunctionStatement,Statement)

FunctionStatement::FunctionStatement()
: mIsTopLevel(true)
{
}

FunctionStatement::FunctionStatement(const std::string &name)
	: mName(name)
	, mIsTopLevel(false)
{
}

FunctionStatement* FunctionStatement::GetParentFunction()
{
	Statement *p = GetParent();
	while (p && !p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		p = p->GetParent();
	return p ? static_cast<FunctionStatement*>(p) : nullptr;
}

int FunctionStatement::ParseParamList(SimpleCScriptEngContext *context)
{
	int parseResult;
	Symbol symbol;

	if ((parseResult = context->GetNextSymbol(symbol)) != 0)
		RETHELP(parseResult);
	if (symbol.symbolOrig == ")")
		return 0;

	context->GoBack();
	for (;;)
	{
		// 形参
		if ((parseResult = context->GetNextSymbol(symbol)) != 0)
			RETHELP(parseResult);
		// 形参必须是符号类型
		if (symbol.type != Symbol::CommonSymbol)
			return -1;

		Param param;
		param.name = symbol.symbolOrig;
		mParamList.emplace_back(param);

		// 类型-1，使得生成代码时，不会生成创建对象的代码，因为
		// 压入的是参数
		if (!RegistNameInContainer(symbol.symbolOrig.c_str(), -1))
		{
			SCRIPT_TRACE("FunctionStatement::ParseParamList: invalid param name.\n");
			return -1;
		}

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

	mParentBlock = parent;

	// 对于非顶级的Function，需要在父块中注册自己的名字
	FunctionStatement *parentFun = GetParentFunction();
	if (!parentFun)
	{
		mName = mEntryFuncName;
	}
	else
	{
		if (!parent->RegistName(mName.c_str(), compiler::KeywordsTransTable::CK_FUNCTION))
			return -1;
	}

	if (parentFun)
	{
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
		context->GoBack();
	}

	if ((parseResult = mFunctionBody.Compile(this, context, true)) < 0)
		return parseResult;
	
	return parseResult;
}

void FunctionStatement::InsertCreateTypeInstructionByDeclType(
	uint32_t declType, GenerateInstructionHelper *giHelper)
{
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

	case KeywordsTransTable::CK_FUNCTION:
		giHelper->Insert_createFunction_Instruction();
		break;

		// 对于压入的参数，无需理会
	case -1:
		break;

	default:
		SCRIPT_TRACE("Invalid data type to create\n");
		exit(1);
	}
}

int FunctionStatement::GenerateLocalVariableInstruction(CompileResult *compileResult)
{
	// 为声明的类型生成将基本数据类型压入堆栈的操作
	GenerateInstructionHelper gih(compileResult);
	for (auto iter = mLocalType.begin(); iter != mLocalType.end(); iter++)
	{
		InsertCreateTypeInstructionByDeclType(*iter, &gih);
	}
	return 0;
}

int FunctionStatement::GenerateInstruction(CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	FunctionStatement *funcParent = GetFunctionParent();
	Statement *parent = funcParent->GetParent();

	uint32_t x;
	uint32_t jumpToFuncEnd;
	uint32_t functionStart;

	if (parent)
	{
		uint32_t l = 0, i = 0;
		if (!GetParent()->FindName(mName.c_str(), l, i))
		{
			return -1;
		}

		// 需要用setval来保存函数对象
		gih.Insert_loadData_Instruction(l, i);
		x = gih.Insert_createFunction_Instruction();
		jumpToFuncEnd = gih.Insert_jump_Instruction(0);

		functionStart = compileResult->SaveCurrentCodePosition();
		runtime::FunctionDesc fd;
		fd.len = 0;
		fd.paramCount = mParamList.size();
		fd.stringId = gih.RegistName(mName.c_str());
		gih.InsertFunctionDesc(&fd);
	}

	// 生成其它代码
	int r;
	if ((r = GenerateLocalVariableInstruction(compileResult)) < 0)
		return r;
	r = mFunctionBody.GenerateInstruction(compileResult);
	if (r < 0)
		return r;

	if (parent)
	{
		uint32_t functionEnd = compileResult->SaveCurrentCodePosition();
		// 函数的长度
		gih.SetCode(functionStart, (functionEnd - functionStart) * 4);
		gih.SetCode(jumpToFuncEnd, functionEnd);
		// 函数的开始点
		gih.SetCode(x, functionStart);

		gih.Insert_setVal_Instruction();
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool FunctionStatement::RegistNameInContainer(const char *name, uint32_t declType)
{
	if (mLocalName.find(name) != mLocalName.end())
		return false;
	mLocalName[name] = mLocalType.size();
	mLocalType.push_back(declType);
	return true;
}

bool FunctionStatement::FindNameInContainer(const char *name, uint32_t &level, uint32_t &index) const
{
	auto iter = mLocalName.find(name);
	if (iter != mLocalName.end()) {
		index = iter->second;
		return true;
	}
	level++;
	const Statement *p = GetParent();
	return p ? p->FindName(name, level, index) : false;
}

uint32_t FunctionStatement::ReservedVariableRoom(uint32_t type) {
	mLocalType.push_back(type);
	return mLocalType.size() - 1;
}
