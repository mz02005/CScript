#include "stdafx.h"
#include "compile.h"

using namespace compiler;

IMPLEMENT_OBJINFO(Statement,objBase)

Statement::Statement()
	: mParentBlock(NULL)
{
}

Statement::~Statement()
{
}

int Statement::GenerateInstruction(CompileResult *compileResult) {
	SCRIPT_TRACE("GenerateInstruction function does not implement by [%s].\n",
		GetThisObjInfo()->className);
	return -1;
}

FunctionStatement* Statement::GetFunctionParent()
{
	Statement *p = this;
	while (p && !p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		p = p->GetParent();
	return p ? static_cast<FunctionStatement*>(p) : NULL;
}

const FunctionStatement* Statement::GetFunctionParent() const
{
	const Statement *p = this;
	while (p && !p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		p = p->GetParent();
	return p ? static_cast<const FunctionStatement*>(p) : NULL;
}

Statement* Statement::isInLoopStatementBlock(uint32_t breakOrContinue)
{
	Statement *p = this;
	while (p)
	{
		uint32_t loopFlags = p->isLoopStatement();
		if (loopFlags)
		{
			if (loopFlags & breakOrContinue)
				return p;
			return NULL;
		}

		if (p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		{
			// 循环之类的，被FunctionStatement截断
			// 说明在当前FunctionStatement中，不存在相匹配的循环代码
			break;
		}
		p = p->GetParent();
	}
	return NULL;
}

bool Statement::RegistName(const char *name, uint32_t type)
{
	Statement *p = this;
	while (p)
	{
		if (p->isInheritFrom(OBJECT_INFO(StatementBlock)))
		{
			return static_cast<StatementBlock*>(p)->RegistNameInBlock(name, type);
		}
		else if (p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		{
			return static_cast<FunctionStatement*>(p)->RegistNameInContainer(name, type);
		}
		else
		{
			p = p->GetParent();
		}
	}
	return false;
}

bool Statement::CheckValidLayer(uint32_t l) const
{
	if (l < 1)
		return true;

	const FunctionStatement *f = GetFunctionParent();
	assert(f);
	for (uint32_t i = 0; i < l; i++)
	{
		f = f->GetParent()->GetFunctionParent();
		if (!f)
			return false;
	}
	return f->isTopLevelFun();
}

bool Statement::FindName(const char *name, uint32_t &l, uint32_t &i) const
{
	const Statement *p = this;
	while (p)
	{
		if (p->isInheritFrom(OBJECT_INFO(StatementBlock)))
		{
			return static_cast<const StatementBlock*>(p)->FindNameInBlock(name, l, i);
		}
		else if (p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		{
			if (p != this)
				l++;
			return static_cast<const FunctionStatement*>(p)->FindNameInContainer(name, l, i);
		}
		else
		{
			p = p->GetParent();
		}
	}
	return false;
}
