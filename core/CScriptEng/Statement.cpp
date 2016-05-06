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

StatementBlock* Statement::GetBlockParent()
{
	Statement *statement = this;
	while (statement)
	{
		if (statement->isInheritFrom(OBJECT_INFO(StatementBlock)))
			return static_cast<StatementBlock*>(statement);
		statement = statement->GetParent();
	}
	return nullptr;
}

const StatementBlock* Statement::GetBlockParent() const
{
	const Statement *statement = this;
	while (statement)
	{
		if (statement->isInheritFrom(OBJECT_INFO(StatementBlock)))
			return static_cast<const StatementBlock*>(statement);
		statement = statement->GetParent();
	}
	return nullptr;
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

		p = p->GetParent();
	}
	return nullptr;
}
