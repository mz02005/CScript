#include "stdafx.h"
#include "compile.h"
#include "vm.h"

using namespace compiler;

IMPLEMENT_OBJINFO(DeclareStatement,Statement)

DeclareStatement::DeclareStatement()
: mDeclType(compiler::KeywordsTransTable::CK_INT)
{
}

DeclareStatement::~DeclareStatement()
{
	for (auto iter = mInfoList.begin(); iter != mInfoList.end(); iter++)
		delete (*iter).mSetValueExpression;
	mInfoList.clear();
}

int DeclareStatement::Compile(Statement *parent, SimpleCScriptEngContext *context)
{
	int r = -1;
	Symbol symbol;

	mParentBlock = parent;
	while (1)
	{
		r = context->GetNextSymbol(symbol);
		if (r != 0) {
			r = -1;
			break;
		}
		if (symbol.type != Symbol::CommonSymbol) {
			r = -1;
			break;
		}

		std::string name = symbol.symbolOrig;

		r = context->GetNextSymbol(symbol);
		if (r != 0) {
			r = -1;
			break;
		}
		if (symbol.type != Symbol::terminalChar) {
			r = -1;
			break;
		}
		if (symbol.symbolOrig == ";")
		{
			DeclareStatement::DeclareInfo di;
			di.mSetValueExpression = nullptr;
			di.mVarName = name;
			mInfoList.push_back(std::move(di));
			break;
		}
		else if (symbol.symbolOrig == ",")
		{
			DeclareStatement::DeclareInfo di;
			di.mSetValueExpression = nullptr;
			di.mVarName = name;
			mInfoList.push_back(std::move(di));
		}
		else if (symbol.symbolOrig == "=")
		{
			char ec;
			PostfixExpression *exp = new PostfixExpression;
			r = context->ParseExpressionEndWith(ec, exp, ",;");
			if (r != 0) {
				delete exp;
				r = -1;
				break;
			}
			DeclareStatement::DeclareInfo di;
			di.mSetValueExpression = exp;
			di.mVarName = name;
			mInfoList.push_back(std::move(di));
			if (ec == ';')
				break;
		}
		else
		{
			r = -1;
			break;
		}
	}
	
	for (auto iter = mInfoList.begin(); iter != mInfoList.end(); iter++)
	{
		if (!RegistName(iter->mVarName.c_str(), mDeclType))
			return -1;
		if (iter->mSetValueExpression)
		{
			if ((r = iter->mSetValueExpression->CreateExpressionTree()) < 0)
				return r;
		}
	}
	
	return r;
}

int DeclareStatement::GenerateInstruction(CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	int r = 0;
	for (auto iter = mInfoList.begin(); iter != mInfoList.end(); iter++)
	{
		if (iter->mSetValueExpression)
		{
			uint32_t l = 0, i = 0;

			// 手动加一条赋值语句进去

			if (!FindName(iter->mVarName.c_str(), l, i)
				|| !CheckValidLayer(l))
			{
				SCRIPT_TRACE("Varaible [%s] not found\n", iter->mVarName.c_str());
				return -1;
			}
			gih.Insert_loadData_Instruction(l, i);

			if ((r = iter->mSetValueExpression->GenerateInstruction(this, compileResult)) < 0)
				return r;

			gih.Insert_setVal_Instruction();
			// 处理多压进去的那个对象
			gih.Insert_pop_Instruction();
		}
	}
	return r;
}
