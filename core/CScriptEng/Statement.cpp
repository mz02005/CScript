#include "stdafx.h"
#include "compile.h"

using namespace compiler;


IMPLEMENT_OBJINFO(Statement,objBase)

Statement::Statement()
	: mParentBlock(nullptr)
{
}

Statement::~Statement()
{
}

FunctionStatement* Statement::GetFunctionParent()
{
	Statement *p = this;
	while (p && !p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		p = p->GetParent();
	return p ? static_cast<FunctionStatement*>(p) : nullptr;
}

const FunctionStatement* Statement::GetFunctionParent() const
{
	const Statement *p = this;
	while (p && !p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		p = p->GetParent();
	return p ? static_cast<const FunctionStatement*>(p) : nullptr;
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
			return nullptr;
		}

		if (p->isInheritFrom(OBJECT_INFO(FunctionStatement)))
		{
			// 循环之类的，被FunctionStatement截断
			// 说明在当前FunctionStatement中，不存在相匹配的循环代码
			break;
		}
		p = p->GetParent();
	}
	return nullptr;
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
