#include "StdAfx.h"
#include "compile.h"
#include "notstd/notstd.h"
#include "rtlib.h"
#include <regex>

using namespace compiler;

///////////////////////////////////////////////////////////////////////////////

const KeywordsTransTable::KeywordsEntry KeywordsTransTable::mKeywords[] =
{
	{ "int", CK_INT, },
	{ "uint", CK_UINT, },
	{ "float", CK_FLOAT, },
	{ "string", CK_STRING, },
	//{ "unsigned", CK_UNSIGNED, },
	{ "char", CK_CHAR, },
	{ "uchar", CK_UCHAR, },
	{ "short", CK_SHORT, },
	{ "ushort", CK_USHORT, },
	{ "if", CK_IF, },
	{ "else", CK_ELSE, },
	{ "while", CK_WHILE, },
	{ "do", CK_DO, },
	{ "break", CK_BREAK, },
	{ "continue", CK_CONTINUE, },
	{ "for", CK_FOR, },
	{ "switch", CK_SWITCH, },
	{ "case", CK_CASE, },
	{ "default", CK_DEFAULT, },
	{ "array", CK_ARRAY, },
	{ "function", CK_FUNCTION, },
	{ "debugbreak", CK_DEBUGBREAK, },
	{ "return", CK_RETURN, },
};

const uint32_t KeywordsTransTable::mKeywordsCount = sizeof(KeywordsTransTable::mKeywords) / sizeof(KeywordsTransTable::mKeywords[0]);

KeywordsTransTable::TransNode::TransNode(char c)
	: theChar(c)
	, keywordsId(0)
{
	memset(theNext, 0, sizeof(theNext));
}

KeywordsTransTable::TransNode::~TransNode()
{
	int count = sizeof(theNext) / sizeof(theNext[0]);
	for (int i = 0; i < count; i++)
	{
		if (theNext[i])
			delete theNext[i];
	}
}

KeywordsTransTable::KeywordsTransTable()
	: mRoot(new TransNode(0))
	, mCurrent(mRoot)
{
}

KeywordsTransTable::~KeywordsTransTable()
{
	Term();
	delete mRoot;
}

void KeywordsTransTable::Init()
{
	TransNode *node;
	for (uint32_t i = 0; i < mKeywordsCount; i++)
	{
		const char *p = mKeywords[i].keywords;
		node = mRoot->theNext[(unsigned char)*p];
		if (*p == '\0')
			continue;
		if (!node)
		{
			node = new TransNode(*p);
			mRoot->theNext[(unsigned char)*p] = node;
		}
		p++;
		for (; *p != '\0'; p++)
		{
			TransNode *&n = node->theNext[(unsigned char)*p];
			if (!n)
				n = new TransNode(*p);
			node = n;
		}
		node->keywordsId = mKeywords[i].id;
	}
	mCurrent = mRoot;
}

void KeywordsTransTable::Term()
{
	delete mRoot;
	mRoot = new TransNode(0);
}

const KeywordsTransTable::TransNode* KeywordsTransTable::GoNext(
	char c)
{
	assert(mCurrent);
	mCurrent = mCurrent->theNext[(unsigned char)c];
	return mCurrent;
}

void KeywordsTransTable::Reset()
{
	mCurrent = mRoot;
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJINFO(ExpressionNode,objBase)

ExpressionNode::ExpressionNode()
	: mParent(nullptr)
{
}

ExpressionNode::~ExpressionNode()
{
}

void ExpressionNode::BeforeAddToPostfixExpression(PostfixExpression *postfixExpression)
{
}

void ExpressionNode::AfterAddToPostfixExpression(PostfixExpression *postfixExpression)
{
}

///////////////////////////////////////////////////////////////////////////////

Symbol::Symbol()
	: type(CommonSymbol)
	, keywordsType(KeywordsTransTable::CK_NULL)
{
	codePos.column = codePos.line = 1;
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJINFO(SymbolExpressionNode, ExpressionNode)

SymbolExpressionNode::SymbolExpressionNode(const Symbol &symbol)
{
	mSymbol.type = symbol.type;
	mSymbol.symbolOrig = symbol.symbolOrig;
}

void SymbolExpressionNode::DebugPrint() const
{
	if (mSymbol.type == Symbol::String)
		printf("\"%s\" ", mSymbol.symbolOrig.c_str());
	else
		printf("%s ", mSymbol.symbolOrig.c_str());
}

int SymbolExpressionNode::GenerateInstruction(Statement *statement, CompileResult *compileResult)
{
	StatementBlock *sb;
	GenerateInstructionHelper gih(compileResult);

	switch (mSymbol.type)
	{
	case Symbol::constInt:
		gih.Insert_createInt_Instruction(
			atoi(mSymbol.symbolOrig.c_str()));
		break;

	case Symbol::constFloat:
		gih.Insert_createFloat_Instruction((float)atof(mSymbol.symbolOrig.c_str()));
		break;

	case Symbol::String:
		gih.Insert_createString_Instruction(mSymbol.symbolOrig.c_str());
		break;

	default:
		// 如果上一级是.操作符，则优先考虑
		if (mParent && mParent->isInheritFrom(OBJECT_INFO(OperatorExpressionNode))
			&& static_cast<OperatorExpressionNode*>(mParent)->mOperatorEntry->operatorId == OP_fullstop)
		{
			//static_cast<OperatorExpressionNode*>(mParent)->mDotOperatorStrId = 
			//	compileResult->GetStringData()->RegistString(mSymbol.symbolOrig);
		}
		else
		{
			sb = statement->GetBlockParent();
			assert(sb);
			uint32_t l, i;
			if (!sb->FindName(mSymbol.symbolOrig.c_str(), l, i))
			{
				SCRIPT_TRACE("variable or symbol [%s] not defined.\n", mSymbol.symbolOrig.c_str());
				return -13;
			}
			if (l >= 65536)
			{
				SCRIPT_TRACE("invalid stack frame level [%u].\n", l);
				return -14;
			}
			gih.Insert_copyAtFrame_Instruction(l, i);
		}
		break;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJINFO(SubProcCallExpression, OperatorExpressionNode)

SubProcCallExpression::SubProcCallExpression(const OperatorHelper::OperatorTableEntry *operatorEntry)
: OperatorExpressionNode(operatorEntry)
{
}

SubProcCallExpression::~SubProcCallExpression()
{
	std::for_each(mRealParams.begin(), mRealParams.end(), [](PostfixExpression *n)
	{
		delete n;
	});
}

void SubProcCallExpression::DebugPrint() const
{
	//printf("%s() ", mSubProcName.c_str());
}

void SubProcCallExpression::BeforeAddToPostfixExpression(PostfixExpression *postfixExpression)
{
	//postfixExpression->RemoveTail();
}

int SubProcCallExpression::GenerateInstruction(Statement *statement, CompileResult *compileResult)
{
	if (mRealParams.size() >= 65536)
	{
		SCRIPT_TRACE("Function call param out of range.\n");
		return -1;
	}

	GenerateInstructionHelper gih(compileResult);

	int r;
	for (auto iter = mRealParams.begin(); iter != mRealParams.end(); iter++)
	{
		if ((r = (*iter)->CreateExpressionTree()) < 0)
			return r;
		if ((r = (*iter)->GenerateInstruction(statement, compileResult)) < 0)
			return r;
	}

	if (mRealParams.size() >= 65536)
	{
		SCRIPT_TRACE("doCall has too many parameters.\n");
		return -1;
	}
	gih.Insert_setParamCount_Instruction(mRealParams.size());
	gih.Insert_doCall_Instruction();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJINFO(QuestionExpression,OperatorExpressionNode)

QuestionExpression::QuestionExpression(const OperatorHelper::OperatorTableEntry *operatorEntry)
	: OperatorExpressionNode(operatorEntry)
	, mJudgement(nullptr)
	, mTrueExpression(nullptr)
	, mFalseExpression(nullptr)
{
}

QuestionExpression::~QuestionExpression()
{
	if (mFalseExpression)
		delete mFalseExpression;
	if (mTrueExpression)
		delete mTrueExpression;
	if (mJudgement)
		delete mJudgement;
}

int QuestionExpression::GenerateInstruction(Statement *statement, CompileResult *compileResult)
{
	int r;
	GenerateInstructionHelper gih(compileResult);
	assert(mJudgement && mTrueExpression && mFalseExpression);

	if ((r = PostfixExpression::GenerateInstruction(mJudgement, statement, compileResult)) < 0)
		return r;

	uint32_t falsePosToFill = gih.Insert_jz_Instruction(0);

	if ((r = PostfixExpression::GenerateInstruction(mTrueExpression, statement, compileResult)) < 0)
		return r;


	uint32_t gotoEndToFill = gih.Insert_jump_Instruction(0);

	gih.SetCode(falsePosToFill, compileResult->SaveCurrentCodePosition());
	if ((r = PostfixExpression::GenerateInstruction(mFalseExpression, statement, compileResult)) < 0)
		return r;

	gih.SetCode(gotoEndToFill, compileResult->SaveCurrentCodePosition());
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJINFO(ArrayAccessExpress, OperatorExpressionNode)

ArrayAccessExpress::ArrayAccessExpress(const OperatorHelper::OperatorTableEntry *operatorEntry)
: OperatorExpressionNode(operatorEntry)
{
}

ArrayAccessExpress::~ArrayAccessExpress()
{
	if (mAccessPosExpression)
		delete mAccessPosExpression;
}

void ArrayAccessExpress::DebugPrint() const
{
}

void ArrayAccessExpress::BeforeAddToPostfixExpression(PostfixExpression *postfixExpression)
{
	//postfixExpression->RemoveTail();
}

int ArrayAccessExpress::GenerateInstruction(Statement *statement, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);
	int result;

	if ((result = mAccessPosExpression->GenerateInstruction(statement, compileResult)) < 0)
		return result;

	gih.Insert_getIndex_Instruction();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJINFO(OperatorExpressionNode,objBase)

OperatorExpressionNode::OperatorExpressionNode(
	const OperatorHelper::OperatorTableEntry *operatorEntry)
	: mHasConstruct(false)
	, mOperatorEntry(operatorEntry)
	, mHasGenerate(false)
	, mDoNotDeleteSubOnDestory(false)
	//, mDotOperatorStrId(0)
{
	mSubNodes[0] = mSubNodes[1] = nullptr;
}

OperatorExpressionNode::~OperatorExpressionNode()
{
	if (!mDoNotDeleteSubOnDestory)
	{
		if (mSubNodes[0])
			delete mSubNodes[0];
		if (mSubNodes[1])
			delete mSubNodes[1];
	}
}

void OperatorExpressionNode::DebugPrint() const
{
	printf("%s ", mOperatorEntry->operString);
}

int OperatorExpressionNode::GenerateInstruction(Statement *statement, CompileResult *compileResult)
{
	GenerateInstructionHelper gih(compileResult);

	switch (mOperatorEntry->operatorId)
	{
	case OP_add:
		gih.Insert_add_Instruction();
		break;

	case OP_sub:
		gih.Insert_sub_Instruction();
		break;

	case OP_mul:
		gih.Insert_mul_Instruction();
		break;

	case OP_div:
		gih.Insert_div_Instruction();
		break;

	case OP_mod:
		gih.Insert_mod_Instruction();
		break;

	case OP_logic_and:
		gih.Insert_logicAnd_Instruction();
		break;
	case OP_logic_or:
		gih.Insert_logicOr_Instruction();
		break;
	case OP_logic_not:
		gih.Insert_logicNot_Instruction();
		break;

	case OP_bitwise_xor:
		gih.Insert_bitwiseXOR_Instruction();
		break;
	case OP_bitwise_and:
		gih.Insert_bitwiseAnd_Instruction();
		break;
	case OP_bitwise_or:
		gih.Insert_bitwiseOr_Instruction();
		break;
	case OP_bitwise_not:
		gih.Insert_bitwiseNot_Instruction();
		break;

		// 对于+=之类的自加运算，可以产生如下的代码
		// push 1
		// push 0
		// push 2
		// add（这里应该对应实际的运算）
		// setval
		// pop
		// pop
		// TODO: 可能还需要加上一句pop，明天看看是不是所有的Statement后面都需要这个
	case OP_add_setval:
	case OP_sub_setval:
	case OP_mul_setval:
	case OP_div_setval:
	case OP_mod_setval:
		do {
			gih.Insert_push_Instruction(1);
			gih.Insert_push_Instruction(0);
			gih.Insert_push_Instruction(2);

			switch (mOperatorEntry->operatorId)
			{
			case OP_add_setval: gih.Insert_add_Instruction(); break;
			case OP_sub_setval: gih.Insert_sub_Instruction(); break;
			case OP_mul_setval: gih.Insert_mul_Instruction(); break;
			case OP_div_setval: gih.Insert_div_Instruction(); break;
			case OP_mod_setval: gih.Insert_mod_Instruction(); break;
			}

			gih.Insert_setVal_Instruction();
			gih.Insert_pop_Instruction();
			gih.Insert_pop_Instruction();
		} while (0);
		//SCRIPT_TRACE("Instruction does not generate.\n");
		break;

	case OP_setval:
		gih.Insert_setVal_Instruction();
		break;

	case OP_docall:
		gih.Insert_doCall_Instruction();
		break;

		// 下面这些是比较运算
	case OP_lessthan:
		gih.Insert_less_Instruction();
		break;
	case OP_greaterthan:
		gih.Insert_greater_Instruction();
		break;
	case OP_isnotequal:
		gih.Insert_notEqual_Instruction();
		break;
	case OP_isequal:
		gih.Insert_equal_Instruction();
		break;
	case OP_lessthan_equalto:
		gih.Insert_lessEqual_Instruction();
		break;
	case OP_greaterthan_equalto:
		gih.Insert_greaterEqual_Instruction();
		break;

	case OP_neg:
		do {
			// 插入*-1的代码
			gih.Insert_createFloat_Instruction(-1.f);
			gih.Insert_mul_Instruction();
		} while (0);
		break;

	//case OP_fullstop:
	//	code.push_back(runtime::VM_getMember);
	//	code.push_back(runtime::CommonInstruction(mDotOperatorStrId));
	//	break;

	case OP_getmember:
		gih.Insert_getMember_Instruction(mMemberName.c_str());
		break;

	default:
		SCRIPT_TRACE("operator has not implement. [%s]\n", mOperatorEntry->operString);
		return -15;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

PostfixExpression::PostfixExpression()
	: mGrammaTreeRoot(nullptr)
{
}

PostfixExpression::~PostfixExpression()
{
	Clear();
	if (mGrammaTreeRoot)
	{
		RemoveGrammaTree(mGrammaTreeRoot);
		mGrammaTreeRoot = nullptr;
	}
}

void PostfixExpression::Clear()
{
	std::for_each(mPostfixExpression.begin(), mPostfixExpression.end(),
		[](ExpressionNode *n)
	{
		delete n;
	});
	mPostfixExpression.clear();
}

void PostfixExpression::OnRemoveSingleGrammaTreeNode(
	ExpressionNode *n, std::list<ExpressionNode*> &l)
{
	if (n->isInheritFrom(OBJECT_INFO(SymbolExpressionNode)))
	{
		delete n;
	}
	else if (n->isInheritFrom(OBJECT_INFO(OperatorExpressionNode)))
	{
		OperatorExpressionNode *p = static_cast<OperatorExpressionNode*>(n);
		p->mDoNotDeleteSubOnDestory = true;
		if (p->mSubNodes[0])
			l.push_back(p->mSubNodes[0]);
		if (p->mSubNodes[1])
			l.push_back(p->mSubNodes[1]);
		delete p;
	}
}

void PostfixExpression::RemoveGrammaTree(ExpressionNode *root)
{
	std::list<ExpressionNode*> toRemoveList;
	
	OnRemoveSingleGrammaTreeNode(root, toRemoveList);
	while (!toRemoveList.empty())
	{
		ExpressionNode *p = toRemoveList.front();
		toRemoveList.pop_front();

		OnRemoveSingleGrammaTreeNode(p, toRemoveList);
	}
}

void PostfixExpression::RemoveTail()
{
	assert(!mPostfixExpression.empty());
	delete mPostfixExpression.back();
	mPostfixExpression.pop_back();
}

void PostfixExpression::AddNode(ExpressionNode *subNode)
{
	subNode->BeforeAddToPostfixExpression(this);
	mPostfixExpression.push_back(subNode);
	subNode->AfterAddToPostfixExpression(this);
}

void PostfixExpression::DebugPrint() const
{
	std::for_each(mPostfixExpression.begin(), mPostfixExpression.end(),
		[](ExpressionNode *n)
	{
		n->DebugPrint();
	});
}

bool PostfixExpression::CalcNode(ExpressionNode *n, int &val)
{
	if (n->isInheritFrom(OBJECT_INFO(OperatorExpressionNode)))
	{
		int val1, val2;
		OperatorExpressionNode *p = static_cast<OperatorExpressionNode*>(n);
		if (!p->mOperatorEntry->CalcIntConst)
			return false;
		if (p->mSubNodes[1])
		{
			assert(p->mOperatorEntry->participator == 2);
			if (!CalcNode(p->mSubNodes[0], val1))
				return false;
			if (!CalcNode(p->mSubNodes[1], val2))
				return false;
			if (!(*p->mOperatorEntry->CalcIntConst)(val, val1, val2))
			{
				throw std::bad_cast("mod or div by zero!");
				return false;
			}
			return true;
		}

		if (!CalcNode(p->mSubNodes[0], val1))
			return false;
		val2 = 0;
		if (!(*p->mOperatorEntry->CalcIntConst)(val, val1, val2))
		{
			throw std::bad_cast("mod or div by zero!");
			return false;
		}
		return true;
	}

	assert(n->isInheritFrom(OBJECT_INFO(SymbolExpressionNode)));
	SymbolExpressionNode *sn = static_cast<SymbolExpressionNode*>(n);
	int type = sn->GetSymbol().type;
	if (type == Symbol::constInt)
		val = atoi(sn->GetSymbol().symbolOrig.c_str());
	else if (type == Symbol::SingleChar)
		val = (int)sn->GetSymbol().symbolOrig[0];
	else
		return false;
	return true;
}

bool PostfixExpression::IsConstIntergerExpression(int &val)
{
	if (CreateExpressionTree() < 0)
		return false;

	if (!CalcNode(mGrammaTreeRoot, val))
		return false;
	
	return true;
}

int PostfixExpression::GenerateInstruction(ExpressionNode *root, Statement *statement, CompileResult *compileResult)
{
	int r = 0;
	std::list<ExpressionNode*> parseList;
	
	if (root->isInheritFrom(OBJECT_INFO(QuestionExpression)))
	{
		if ((r = root->GenerateInstruction(statement, compileResult)) < 0)
			return r;
	}
	else if (HasChildren(root))
	{
		parseList.push_back(root);
		OperatorExpressionNode *node = static_cast<OperatorExpressionNode*>(root);
		node->mHasGenerate = true;
		if (node->mSubNodes[1])
			parseList.push_back(node->mSubNodes[1]);
		parseList.push_back(node->mSubNodes[0]);
	}
	else
	{
		if ((r = root->GenerateInstruction(statement, compileResult)) < 0)
			return r;
	}

	// 使用后序遍历来从这颗语法树生成代码
	while (!parseList.empty())
	{
		ExpressionNode *e = parseList.back();
		if (e->isInheritFrom(OBJECT_INFO(QuestionExpression)))
		{
			if ((r = e->GenerateInstruction(statement, compileResult)) < 0)
				return r;
		}
		else if (HasChildren(e))
		{
			OperatorExpressionNode *node = static_cast<OperatorExpressionNode*>(e);
			if (!node->mHasGenerate)
			{
				node->mHasGenerate = true;
				if (node->mSubNodes[1])
					parseList.push_back(node->mSubNodes[1]);
				parseList.push_back(node->mSubNodes[0]);
			}
			else
			{
				parseList.pop_back();
				if ((r = node->GenerateInstruction(statement, compileResult)) < 0)
					return r;
			}
		}
		else
		{
			parseList.pop_back();
			if ((r = e->GenerateInstruction(statement, compileResult)) < 0)
				return r;
		}
	}
	
	return r;
}

int PostfixExpression::GenerateInstruction(Statement *statement, CompileResult *compileResult)
{
	int val, parseResult;

	if (!mGrammaTreeRoot)
	{
		if ((parseResult = CreateExpressionTree()) < 0)
			return parseResult;
	}

	try {
		if (IsConstIntergerExpression(val))
		{
			GenerateInstructionHelper gih(compileResult);
			gih.Insert_createInt_Instruction(val);
			return 0;
		}
	}
	catch (...)
	{
		return -1;
	}
	return GenerateInstruction(mGrammaTreeRoot, statement, compileResult);
}

bool PostfixExpression::isGetMember(OperatorExpressionNode *n, 
	ExpressionNode *left, ExpressionNode *right) const
{
	if (n->mOperatorEntry->operatorId != OP_fullstop)
		return false;

	return right->GetThisObjInfo() == OBJECT_INFO(SymbolExpressionNode);
}

int PostfixExpression::CreateExpressionTree()
{
	if (mGrammaTreeRoot)
		return 0;

	int r = 0;
	uint32_t stackPos = 0;
	std::list<ExpressionNode*> parseList;
	for (auto iter = mPostfixExpression.begin(); iter != mPostfixExpression.end();)
	{
		ExpressionNode *expNode = *iter++;
		if (expNode->isInheritFrom(OBJECT_INFO(OperatorExpressionNode)))
		{
			OperatorExpressionNode *operExpNode = static_cast<OperatorExpressionNode*>(expNode);
			if (operExpNode->mOperatorEntry->participator > static_cast<int>(parseList.size()))
			{
				SCRIPT_TRACE("Create grammer tree fail\n");
				return -1;
			}

			if (operExpNode->mOperatorEntry->participator == 2)
			{
				if (operExpNode->mOperatorEntry->operatorId == OP_questionmark)
				{
					if (parseList.back()->GetThisObjInfo() != OBJECT_INFO(OperatorExpressionNode)
						|| static_cast<OperatorExpressionNode*>(parseList.back())->mOperatorEntry->operatorId != OP_colon)
						return -1;
					OperatorExpressionNode *colonNode = static_cast<OperatorExpressionNode*>(parseList.back());
					parseList.pop_back();
					//if (!parseList.back()->isInheritFrom(OBJECT_INFO(ExpressionNode)))
					//	return -1;
					QuestionExpression *questionNode = new QuestionExpression(operExpNode->mOperatorEntry);
					questionNode->mParent = colonNode->mParent;
					questionNode->mJudgement = parseList.back();
					parseList.pop_back();
					questionNode->mTrueExpression = colonNode->mSubNodes[0];
					questionNode->mFalseExpression = colonNode->mSubNodes[1];
					colonNode->mSubNodes[0] = colonNode->mSubNodes[1] = nullptr;
					delete colonNode;
					parseList.push_back(questionNode);
				}
				else
				{
					ExpressionNode *left = (*----parseList.end());
					ExpressionNode *right = parseList.back();

					if (isGetMember(operExpNode, left, right))
					{
						parseList.pop_back();
						parseList.pop_back();

						operExpNode->mSubNodes[0] = left;
						operExpNode->mOperatorEntry = OperatorHelper::GetOperatorEntryByOperatorString("getmember");
						operExpNode->mMemberName = static_cast<SymbolExpressionNode*>(right)->GetSymbol().symbolOrig;

						parseList.push_back(operExpNode);
						delete right;
					}
					else
					{
						parseList.pop_back();
						parseList.pop_back();
						parseList.push_back(operExpNode);
						operExpNode->mSubNodes[0] = left;
						operExpNode->mSubNodes[1] = right;
					}
				}
			}
			else
			{
				operExpNode->mSubNodes[0] = parseList.back();
				parseList.pop_back();
				parseList.push_back(operExpNode);
			}
		}
		else
		{
			parseList.push_back(expNode);
		}
	}
	if (r >= 0 && parseList.size() == 1)
	{
		mGrammaTreeRoot = parseList.front();
		parseList.clear();
		mPostfixExpression.clear();
		return r;
	}

	//Clear();
	mPostfixExpression.clear();
	return r < 0 ? r : -1;
}

///////////////////////////////////////////////////////////////////////////////

void OperatorHelper::InitEntryTable()
{
	for (const OperatorTableEntry *p = mOperTable; p->operString != NULL; p++)
	{
		mOperatorEntryTable[p->operString] = p;
	}
}
void OperatorHelper::TermEntryTable()
{
	mOperatorEntryTable.clear();
}

const OperatorHelper::OperatorTableEntry* OperatorHelper::GetOperatorEntryByOperatorString(const char *s)
{
	auto r = mOperatorEntryTable.find(s);
	if (r != mOperatorEntryTable.end())
		return r->second;
	return NULL;
}

std::map<std::string, const OperatorHelper::OperatorTableEntry*> OperatorHelper::mOperatorEntryTable;

class ConstIntegerCalucator
{
public:
	static bool mod(int &a, int l, int r) { if (!r) { SCRIPT_TRACE("mod by zero!\n"); return false; } a = l % r; return true; }
	static bool bitwise_xor(int &a, int l, int r) { a = l ^ r; return true; }
	static bool bitwise_and(int &a, int l, int r) { a = l & r; return true; }
	static bool bitwise_not(int &a, int l, int r) { a = ~l; return true; }
	static bool bitwise_or(int &a, int l, int r) { a = l | r; return true; }
	static bool mul(int &a, int l, int r) { a = l * r; return true; }
	static bool sub(int &a, int l, int r) { a = l - r; return true; }
	static bool add(int &a, int l, int r) { a = l + r; return true; }
	static bool div(int &a, int l, int r) { if (!r) { SCRIPT_TRACE("divided by zero!\n"); return false; } a = l / r; return true; }
	static bool lessthan(int &a, int l, int r) { a = l < r; return true; }
	static bool greaterthan(int &a, int l, int r) { a = l > r; return true; }
	static bool isnotequal(int &a, int l, int r) { a = l != r; return true; }
	static bool iseuqal(int &a, int l, int r) { a = l == r; return true; }
	static bool lessthan_equalto(int &a, int l, int r) { a = l <= r; return true; }
	static bool greaterthan_equalto(int &a, int l, int r) { a = l >= r; return true; }
	static bool logic_not(int &a, int l, int r) { a = !l; return true; }
	static bool logic_and(int &a, int l, int r) { a = l && r; return true; }
	static bool logic_or(int &a, int l, int r) { a = l || r; return true; }
	static bool commas(int &a, int l, int r) { a = r; return true; }
	static bool neg(int &a, int l, int r) { a = -l; return true; }
};

const OperatorHelper::OperatorTableEntry OperatorHelper::mOperTable[] =
{

	{ OP_mod,"%", 2, 40, &ConstIntegerCalucator::mod, },
	{ OP_bitwise_xor,"^", 2, 40, &ConstIntegerCalucator::bitwise_xor, },
	{ OP_bitwise_and,"&", 2, 40, &ConstIntegerCalucator::bitwise_and, },
	{ OP_bitwise_not, "~", 1, 100, &ConstIntegerCalucator::bitwise_not, 1, },
	{ OP_bitwise_or,"|", 2, 20, &ConstIntegerCalucator::bitwise_or, },
	{ OP_mul,"*", 2, 50, &ConstIntegerCalucator::mul, },
	{ OP_sub,"-", 2, 40, &ConstIntegerCalucator::sub, },
	{ OP_add,"+", 2, 40, &ConstIntegerCalucator::add, },
	{ OP_div,"/", 2, 50, &ConstIntegerCalucator::div, },
	{ OP_setval,"=", 2, 10, nullptr, 1, },
	{ OP_lessthan, "<", 2, 20, &ConstIntegerCalucator::lessthan, },
	{ OP_greaterthan, ">", 2, 20, &ConstIntegerCalucator::greaterthan, },

	{ OP_isnotequal, "!=", 2, 20, &ConstIntegerCalucator::isnotequal, },
	{ OP_mod_setval, "%=", 2, 20, },
	{ OP_bitwise_xor_setval, "^=", 2, 20, },
	{ OP_bitwise_and_setval, "&=", 2, 20, },
	{ OP_bitwise_or_setval, "|=", 2, 20, },
	{ OP_mul_setval, "*=", 2, 20, },
	{ OP_add_setval, "+=", 2, 20, },
	{ OP_sub_setval, "-=", 2, 20, },
	{ OP_div_setval, "/=", 2, 20, },
	{ OP_isequal, "==", 2, 20, &ConstIntegerCalucator::iseuqal, },
	{ OP_lessthan_equalto, "<=", 2, 20, &ConstIntegerCalucator::lessthan_equalto, },
	{ OP_greaterthan_equalto, ">=", 2, 20, &ConstIntegerCalucator::greaterthan_equalto, },

	{ OP_logic_not, "!", 1, 100, &ConstIntegerCalucator::logic_not, 1, },
	{ OP_logic_and, "&&", 2, 20, &ConstIntegerCalucator::logic_and, },
	{ OP_logic_or, "||", 2, 20, &ConstIntegerCalucator::logic_or, },

	// 用二元表达式来记录三元表达式
	{ OP_questionmark, "?", 2, 15, },
	{ OP_colon, ":", 2, 16, },

	{ OP_commas, ",", 2, 5, &ConstIntegerCalucator::commas, },

	// 这是一种虚拟的操作，会被替换掉
	{ OP_fullstop, ".", 2, 70, },

	{ OP_getaddress, "getaddressof&", 1, 100, },
	{ OP_getrefof, "getrefof*", 1, 100, },

	// 取负数
	{ OP_neg, "neg-", 1, 40, &ConstIntegerCalucator::neg, },

	// 调用函数对象，就是调用virtual RuntimeObject* DoCall(void *runtimeContext) = 0;
	{ OP_docall, "docall", 1, 70, },
	// 调用索引对象，就是调用virtual RuntimeObject* GetIndex(void *runtimeContext) = 0;
	{ OP_getbyindex, "getbyindex", 1, 70, },
	// 调用索引对象，就是调用virtual RuntimeObject* GetMember(const char *name) = 0;
	{ OP_getmember, "getmember", 1, 70, },

	{ -1, NULL, },
};

///////////////////////////////////////////////////////////////////////////////

ConstStringData::ConstStringData()
{
	// 空字符串是给缺省的字符串变量用的
	RegistString("");
}

void ConstStringData::Clear()
{
	mStringBuffer.clear();
	mIndexTable.clear();
	mConstStringIndexTable.clear();
}

size_t ConstStringData::RegistString(const std::string &str)
{
	auto it = mIndexTable.find(str);
	if (it == mIndexTable.end())
	{
		StringIndex si;
		si.offset = mStringBuffer.size();
		si.size = str.size();
		mConstStringIndexTable.push_back(std::move(si));
		mIndexTable[str] = mConstStringIndexTable.size() - 1;
		mStringBuffer.append(str.c_str(), str.size() + 1);
		return mConstStringIndexTable.size() - 1;
	}
	return it->second;
}

bool ConstStringData::GetString(size_t i, std::string &s) const
{
	if (i >= mConstStringIndexTable.size())
		return false;
	s = &mStringBuffer[mConstStringIndexTable[i].offset];
	return true;
}

int ConstStringData::SaveConstStringDataToFile(FILE *file) const
{
	if (mConstStringIndexTable.size() >= 
		(std::vector<StringIndex>::size_type)((uint32_t)-1))
		return -1;

	// 保存索引表
	uint32_t totalCount = mConstStringIndexTable.size();
	fwrite(&totalCount, sizeof(totalCount), 1, file);
	fwrite(&mConstStringIndexTable[0], sizeof(ConstStringData::StringIndex), totalCount, file);

	// 保存字符常量数据
	uint32_t dataSize = mStringBuffer.size();
	fwrite(&dataSize, sizeof(dataSize), 1, file);
	fwrite(&mStringBuffer[0], 1, mStringBuffer.size(), file);
	
	return 0;
}

int ConstStringData::LoadConstStringDataFromFile(FILE *file)
{
	Clear();

	// 读取索引表
	uint32_t totalCount;
	if (fread(&totalCount, sizeof(totalCount), 1, file) != 1)
		return -1;

	mConstStringIndexTable.resize(totalCount);
	if (fread(&mConstStringIndexTable[0], sizeof(ConstStringData::StringIndex), 
		totalCount, file) != totalCount)
		return -2;

	// 读取常量字符串数据
	uint32_t dataSize;
	if (fread(&dataSize, sizeof(dataSize), 1, file) != 1)
		return -3;
	mStringBuffer.resize(dataSize);
	if (fread(&mStringBuffer[0], 1, dataSize, file) != dataSize)
		return -4;

	// 重建索引
	for (uint64_t i = 0; i < totalCount; i ++)
	{
		StringIndex &index = mConstStringIndexTable[(uint32_t)i];
		if (index.offset + index.size >= dataSize)
			return -5;

		std::string str(mStringBuffer.c_str() + index.offset, 
			mStringBuffer.c_str() + index.offset + index.size);
		if (mIndexTable.find(str) != mIndexTable.end())
		{
			// 出现重复的项，无法忍受
			return -6;
		}
		mIndexTable[str] = (uint32_t)i;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

CompileResult::CompileResult(ConstStringData *stringData)
	: mConstStringData(stringData)
{
}

CompileResult::~CompileResult()
{
}

void CompileResult::Clear()
{
	mCode.clear();
}

int CompileResult::SaveCodeToFile(FILE *file) const
{
	uint32_t c = mCode.size();
	if (c >= (ScriptCode::size_type)((uint32_t)-1))
		return -1;

	fwrite(&c, sizeof(c), 1, file);
	fwrite(&mCode[0], sizeof(uint32_t), c, file);

	return 0;
}

int CompileResult::LoadCodeFromFile(FILE *file)
{
	uint32_t c;
	if (fread(&c, sizeof(uint32_t), 1, file) != 1
		|| c >= (ScriptCode::size_type)((uint32_t)-1))
		return -1;

	mCode.resize(c);
	if (fread(&mCode[0], sizeof(uint32_t), c, file)
		!= c)
		return -2;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

const char SimpleCScriptEngContext::mTerminalAlpha[] =
{
	' ', '\t', ',', ';', '~', '!', '%', '^', '&', '*', '(', ')', '\'', '\"',
	'-', '+', '=', '|', '\\', '.', '<', '>', '/', '?', ':', '[', ']',
	'{', '}',
	'\r', '\n',
};

// 这里没有把=本身算进去
const char SimpleCScriptEngContext::mDupTermAlpha[] = 
{
	'&', '+', '-', '|',
};

const char SimpleCScriptEngContext::mFollowWithEqual[] =
{
	'!', '%', '^', '&', '*', '+', '-', '/', '=', '|', '<', '>', 
};

std::set<char> SimpleCScriptEngContext::mTerminalAlphaSet;
std::set<char> SimpleCScriptEngContext::mMayFollowWithEqualSet;

SimpleCScriptEngContext::SimpleCScriptEngContext()
	: mParseCurrent(nullptr)
	, mState(0)
	, mCurrentStack(mSymbolStack.end())
	, mCompileResult(nullptr)
{
	mKeywordsTransTable.Init();
	mTopLevelBlock.PushName("CreateArray");
	runtime::rtLibHelper::RegistObjNames(&mTopLevelBlock);
}

SimpleCScriptEngContext::~SimpleCScriptEngContext()
{
	mKeywordsTransTable.Term();
}

void SimpleCScriptEngContext::Init()
{
	std::for_each(mTerminalAlpha, mTerminalAlpha + sizeof(mTerminalAlpha) / sizeof(mTerminalAlpha[0]),
		[&](char c)
	{
		mTerminalAlphaSet.insert(c);
	});

	std::for_each(mFollowWithEqual, mFollowWithEqual + sizeof(mFollowWithEqual) / sizeof(SimpleCScriptEngContext::mFollowWithEqual[0]),
		[&](char c)
	{
		mMayFollowWithEqualSet.insert(c);
	});
}

void SimpleCScriptEngContext::Term()
{
	mMayFollowWithEqualSet.clear();
	mTerminalAlphaSet.clear();
}

bool SimpleCScriptEngContext::isBlank(char c)
{
	switch (c)
	{
	case ' ':
	case '\t':
	case '\r':
		return true;

	case '\n':
		mCodePosition.line++;
		return true;
	}
	return false;
}

bool SimpleCScriptEngContext::isTerminalAlpha(char c) const
{
	return mTerminalAlphaSet.find(c) != mTerminalAlphaSet.end();
}

bool SimpleCScriptEngContext::mayFollowWithEqual(char c) const
{
	return mMayFollowWithEqualSet.find(c) != mMayFollowWithEqualSet.end();
}

int SimpleCScriptEngContext::BeginParseSymbol(scriptAPI::ScriptSourceCodeStream *codeStream)
{
	mCodePosition.column = mCodePosition.line = 1;

	mCodeStream = codeStream;
	uint64_t codeSize = mCodeStream->Seek(0, scriptAPI::ScriptSourceCodeStream::End);
	// 限制代码的大小
	if (!codeSize || codeSize >= 1024 * 1024 * 100)
		return -1;
	mCodeStream->Seek(0, scriptAPI::ScriptSourceCodeStream::Begin);
	mSourceCode.resize(static_cast<size_t>(codeSize));
	mCodeStream->Read(reinterpret_cast<uint8_t*>(&mSourceCode[0]), 0, static_cast<int>(codeSize));

	mParseCurrent = mSourceCode.c_str();
	mEnd = mParseCurrent + mSourceCode.size();
	mSymbolStack.clear();
	mCurrentStack = mSymbolStack.begin();
	return 0;
}

inline bool isHexChar(char c)
{
	return (c >= '0' && c <= '9') || ::isalpha(c);
}

// 这个函数假定已经可以确定c是十六进制数值字符
inline int hexStringToVal(char c)
{
	return c < 'A' ? (c - '0') : (c & 0x01 + 9);
}

inline bool isOcxChar(char c)
{
	return c >= '0' && c <= '7';
}

int SimpleCScriptEngContext::GetNextSingleChar(char terminal)
{
	switch (*++mParseCurrent)
	{
	case '\r':
	case '\n':
	case '\0':
		return -1;
	default:
		break;
	}
	if (*mParseCurrent == terminal)
		return 256;
	if (*mParseCurrent == '\\')
	{
		int rc = 0;
		char c = *++mParseCurrent;
		switch (c)
		{
		case 'a':
			return 7;
		case 'b':
			return 8;
		case 'f':
			return 12;
		case 'n':
			return 10;
		case 'r':
			return 13;
		case 't':
			return 9;
		case 'v':
			return 11;
		case '\\':
			return 92;
		case '\'':
			return 39;
		case '\"':
			return 34;
		case '?':
			return 63;
		case '0':
			if (isOcxChar(*++mParseCurrent))
			{
				rc += (*mParseCurrent - '0') * 16;
				if (isOcxChar(*++mParseCurrent))
				{
					rc += (*mParseCurrent - '0') * 8;
					return rc;
				}
				return -1;
			}
			mParseCurrent--;
			return 0;
			// 十六进制字符串
		case 'x':
			rc = *++mParseCurrent;
			if (isHexChar(rc))
			{
				rc = hexStringToVal(rc) * 16;
				if (isHexChar(*++mParseCurrent))
				{
					rc += hexStringToVal(*mParseCurrent);
					return rc;
				}
			}
			return -1;
		default:
			if (*mParseCurrent >= '1' && *mParseCurrent <= '7')
			{
				rc = *mParseCurrent * 64;
				if (isOcxChar(*++mParseCurrent))
				{
					rc += *mParseCurrent * 8;
					if (isOcxChar(*++mParseCurrent))
					{
						rc += *mParseCurrent;
						return rc;
					}
				}
			}
			return -1;
		}
	}
	return (unsigned char)*mParseCurrent;
}

void SimpleCScriptEngContext::GoBack()
{
	if (mCurrentStack == mSymbolStack.begin())
	{
		throw std::exception("GoBack error");
	}
	--mCurrentStack;
}

inline bool is16char(char c)
{
	return (c >= '0' && c <= '9')
		|| ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

// 返回0表示成功
// 返回>0的值表示没有成功解析为数值，且已经回退到了合适的字符位置，应按照原来的方式继续解析
// 返回<0的值表示解析失败
inline int TryGetFloatSymbol(Symbol &symbol, char first, const char *&p, const char *e)
{
	// 当已经处理为8进制数时，如果又读取到了>=8的字符
	// 则认为必须是浮点数，而不能是整数
	bool cannotBeInteger = false;

	int state = (first == '0' ? 10 : 1);
	
	for (;;)
	{
		char c = *p++;
		symbol.symbolOrig += c;

		switch (state)
		{
		case 1:
			if (::isdigit((unsigned char)c))
			{
			}
			else if (c == '.')
				state = 2;
			else if (c == 'e' || c == 'E')
				state = 4;
			else {
				if (cannotBeInteger)
					return -1;

				// 返回整数
				symbol.type = Symbol::constInt;
				symbol.symbolOrig.pop_back();
				p--;
				return 0;
			}
			break;

		case 2:
			if (::isdigit((unsigned char)c))
			{
				state = 3;
			}
			else if (c == 'f' || c == 'F')
			{
				// 确定是浮点了
				symbol.symbolOrig.pop_back();
				symbol.type = Symbol::constFloat;
				return 0;
			}
			else if (c == 'e' || c == 'E')
			{
				state = 4;
			}
			else {
				p -= 2;
				symbol.symbolOrig.resize(symbol.symbolOrig.size() - 2);
				if (cannotBeInteger)
				{
					return -1;
				}
				symbol.type = Symbol::constInt;
				// 看看是不是十六进制数
				if (first == '0')
				{
					symbol.symbolOrig.erase(0, 1);
					uint32_t v = strtoul(symbol.symbolOrig.c_str(), NULL, 8);
					StringHelper::Format(symbol.symbolOrig, "%u", v);
				}
				return 0;
			}
			break;

		case 3:
			if (::isdigit((unsigned char)c))
			{
			}
			else if (c == 'f' || c == 'F')
			{
				symbol.symbolOrig.pop_back();
				symbol.type = Symbol::constFloat;
				return 0;
			}
			else if (c == 'e' || c == 'E')
			{
				state = 4;
			}
			else
			{
				// 由于有了小数点，所以在这里也认为是浮点数
				p--;
				symbol.symbolOrig.pop_back();
				symbol.type = Symbol::constFloat;
				return 0;
			}
			break;

		case 4:
			if (::isdigit((unsigned char)c))
				state = 6;
			else if (c == '+' || c == '-')
				state = 5;
			else
				return 1;
			break;

		case 5:
			if (::isdigit((unsigned char)c))
				state = 6;
			else
				return 1;
			break;

		case 6:
			if (::isdigit((unsigned char)c))
			{
			}
			else if (c == 'F' || c == 'f')
			{
				symbol.symbolOrig.pop_back();
				symbol.type = Symbol::constFloat;
				return 0;
			}
			else
			{
				p--;
				symbol.symbolOrig.pop_back();
				symbol.type = Symbol::constFloat;
				return 0;
			}
			break;

		case 10:
			if (c < '8' && c >= '0')
				state = 30;
			else if (c == 'x' || c == 'X')
				state = 20;
			else if (c == '8' || c == '9')
			{
				cannotBeInteger = true;
				state = 1;
			}
			else if (c == '.')
				state = 2;
			else {
				p--;
				symbol.symbolOrig.pop_back();
				symbol.type = Symbol::constInt;
				return 0;
			}
			break;

		case 20:
			if (is16char(c))
				state = 21;
			else
				return 1;
			break;

		case 21:
			if (is16char(c))
			{
			}
			else
			{
				p--;
				symbol.symbolOrig.pop_back();
				// 删除‘0x两个字符’
				symbol.symbolOrig.erase(0, 2);

				uint32_t v = strtoul(symbol.symbolOrig.c_str(), NULL, 16);
				StringHelper::Format(symbol.symbolOrig, "%u", v);
				symbol.type = Symbol::constInt;
				return 0;
			}
			break;

		case 30:
			if (c >= '0' && c < '8')
			{
			}
			else if (c == '8' || c == '9')
			{
				state = 1;
				cannotBeInteger = true;
			}
			else if (c == '.')
			{
				state = 2;
			}
			else
			{
				p--;
				symbol.symbolOrig.pop_back();
				// 去掉头部的那个0
				symbol.symbolOrig.erase(0, 1);

				uint32_t v = strtoul(symbol.symbolOrig.c_str(), NULL, 8);
				StringHelper::Format(symbol.symbolOrig, "%u", v);
				symbol.type = Symbol::constInt;
				return 0;
			}
			break;

		default:
			return -1;
		}
	}

	return -1;
}

int SimpleCScriptEngContext::GetNextSymbol(Symbol &symbol)
{
	symbol.symbolOrig.clear();
	symbol.type = Symbol::CommonSymbol;

	if (mCurrentStack != mSymbolStack.end())
	{
		symbol = *mCurrentStack++;
		return 0;
	}

	const KeywordsTransTable::TransNode *tn = NULL;
	while (mParseCurrent != mEnd)
	{
		if (!isTerminalAlpha(*mParseCurrent))
		{
			// 在这里就将浮点数解析出来，这样可以删掉后面的浮点数点操作符合并逻辑了
			if (!mState)
			{
				if (isdigit((unsigned char)*mParseCurrent))
				{
					std::string::size_type s = symbol.symbolOrig.size();
					symbol.symbolOrig += *mParseCurrent;
					const char *save = mParseCurrent;
					char first = *mParseCurrent++;
					int tryRet = TryGetFloatSymbol(symbol, first, mParseCurrent, mEnd);
					if (tryRet <= 0)
					{
						if (!tryRet)
						{
							mSymbolStack.push_back(symbol);
							mCurrentStack = mSymbolStack.end();
							symbol.codePos = mCodePosition;
						}
						return tryRet;
					}
					mParseCurrent = save;
					symbol.symbolOrig.erase(s);
				}
			}

			symbol.symbolOrig += *mParseCurrent;

			switch (mState)
			{
			case 0:
				if (isdigit((unsigned char)*mParseCurrent))
				{
					mState = 1;
					symbol.type = Symbol::constInt;
				}
				else if (*mParseCurrent == '_')
				{
					mState = 2;
				}
				else
				{
					if ((tn = mKeywordsTransTable.GoNext(*mParseCurrent)))
					{
						symbol.type = Symbol::Keywords;
						mState = 4;
					}
					else {
						mState = 3;
						symbol.type = Symbol::CommonSymbol;
					}
				}
				break;

			case 1:
				if (!isdigit((unsigned char)*mParseCurrent))
				{
					mState = 3;
					symbol.type = Symbol::CommonSymbol;
				}
				break;

			case 2:
				if (isalpha((unsigned char)*mParseCurrent) || *mParseCurrent == '_')
					mState = 3;
				else
					return -1;
				break;

			case 3:
				if (isalpha((unsigned char)*mParseCurrent) || isalnum((unsigned char)*mParseCurrent) || *mParseCurrent == '_')
					break;
				return -1;

			case 4:
				if ((tn = mKeywordsTransTable.GoNext(*mParseCurrent)) == NULL)
				{
					mState = 3;
				}
				break;
			}
		}
		else
		{
			mKeywordsTransTable.Reset();
			mState = 0;

			if (!symbol.symbolOrig.empty())
				break;

			symbol.type = Symbol::terminalChar;

			// 处理字符
			if (*mParseCurrent == '\'')
			{
				int sc = GetNextSingleChar('\'');
				// 字符不能为空
				if (sc >= 256)
					return -1;
				if (sc < 0)
					return sc;
				if (*++mParseCurrent != '\'')
					return -1;
				symbol.symbolOrig += (char)sc;
				symbol.type = Symbol::SingleChar;
				++mParseCurrent;
				mSymbolStack.push_back(symbol);
				mCurrentStack = mSymbolStack.end();
				symbol.codePos = mCodePosition;
				return 0;
			}

			// 处理字符串
			if (*mParseCurrent == '\"')
			{
				int sc;
				while ((sc = GetNextSingleChar('\"')) >= 0 && sc < 256)
				{
					symbol.symbolOrig += (char)sc;
				}
				if (sc < 0)
					return sc;
				++mParseCurrent;
				mCurrentStack = mSymbolStack.end();
				symbol.codePos = mCodePosition;
				symbol.type = Symbol::String;
				mSymbolStack.push_back(symbol);
				return 0;
			}

			// 处理注释
			if (*mParseCurrent == '/')
			{
				++mParseCurrent;
				if (*mParseCurrent == '/')
				{
					++mParseCurrent;
					while (*mParseCurrent != '\0' && *mParseCurrent != '\n')
						mParseCurrent++;
					continue;
				}
				else if (*mParseCurrent == '*')
				{
					++mParseCurrent;

					while (1)
					{
						while (*mParseCurrent != '\0' && *mParseCurrent != '*')
						{
							if (*mParseCurrent == '\n')
								mCodePosition.line++;
							mParseCurrent++;
						}
						if (*mParseCurrent == '\0')
						{
							// 这里考虑提示错误后，继续往下解析
							return -1;
						}
						if (*++mParseCurrent == '/')
						{
							break;
						}
					}

					++mParseCurrent;
					continue;
				}
				else
				{
					mParseCurrent--;
				}
			}

			if (isBlank(*mParseCurrent))
			{
				mParseCurrent++;
				continue;
			}

			symbol.symbolOrig += *mParseCurrent;
			switch (*mParseCurrent)
			{
			case '&': case '+': case '-': case '|':
				if (*(mParseCurrent + 1) == *mParseCurrent)
				{
					symbol.symbolOrig += *++mParseCurrent;
					++mParseCurrent;
					mSymbolStack.push_back(symbol);
					mCurrentStack = mSymbolStack.end();
					symbol.codePos = mCodePosition;
					return 0;
				}
			}

			if (mayFollowWithEqual(*mParseCurrent))
			{
				if (*(mParseCurrent + 1) == '=')
				{
					symbol.symbolOrig += '=';
					mParseCurrent += 2;
					mSymbolStack.push_back(symbol);
					mCurrentStack = mSymbolStack.end();
					symbol.codePos = mCodePosition;
					return 0;
				}
			}

			mParseCurrent++;
			mSymbolStack.push_back(symbol);
			mCurrentStack = mSymbolStack.end();
			symbol.codePos = mCodePosition;
			return 0;
		}
		mParseCurrent++;
	}
	if (!symbol.symbolOrig.empty())
	{
		if (tn && tn->keywordsId && symbol.type == Symbol::Keywords)
		{
			symbol.keywordsType = tn->keywordsId;
			tn = nullptr;
		}
		else {
			if (symbol.type == Symbol::Keywords)
				symbol.type = Symbol::CommonSymbol;
		}
		//if (!tn || !tn->keywordsId && symbol.type == Symbol::Keywords)
		//	symbol.type = Symbol::CommonSymbol;
		//else
		//	symbol.keywordsType = tn->keywordsId;
		mSymbolStack.push_back(symbol);
		mCurrentStack = mSymbolStack.end();
		symbol.codePos = mCodePosition;
		return 0;
	}
	return 1;
}

HANDLE SimpleCScriptEngContext::Compile(scriptAPI::ScriptSourceCodeStream *codeStream, bool end)
{
	int r = BeginParseSymbol(codeStream);
	if (r < 0)
		return nullptr;
	
	mCompileResult = new CompileResult(&mConstStringData);
	do {
		if ((r = mTopLevelBlock.Compile(NULL, this, true)) < 0)
		{
			SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Compile fail [%d].\n", r);
			break;
		}
		
		GenerateInstructionHelper gih(mCompileResult);
		//uint32_t jp = gih.Insert_jump_Instruction(0);
		//std::pair<uint32_t,uint32_t> t = gih.InsertStringDataToCode("this is a script file.");
		//gih.SetCode(jp, t.first + t.second);
		if ((r = mTopLevelBlock.GenerateInstruction(mCompileResult)) < 0)
		{
			SCRIPT_TRACE_(scriptLog::LogTool::TraceException)("Generate instruction fail [%d]\n", r);
			break;
		}
		if (end)
			gih.Insert_end_Instruction();
	} while (0);
	if (r < 0)
	{
		delete mCompileResult;
		mCompileResult = nullptr;
	}
	return mCompileResult;
}

int SimpleCScriptEngContext::PushName(const char *name)
{
	mTopLevelBlock.PushName(name);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

Operator::~Operator()
{
}

struct CommonOperator : public Operator
{
	virtual ExpressionNode* CreateExpressionNode()
	{
		return new OperatorExpressionNode(mOperatorEntry);
	}
};

struct IndexAccessOperator : public Operator
{
	// 指示索引的算数表达式
	PostfixExpression *mAccessPosExpression;

	int Compile(SimpleCScriptEngContext *context)
	{
		int r = -1;
		char c;
		PostfixExpression *pc = new PostfixExpression;
		r = context->ParseExpressionEndWith(c, pc, "]");
		if (r != 0)
		{
			delete pc;
			return -3;
		}
		mAccessPosExpression = pc;
		return r;
	}

	virtual ExpressionNode* CreateExpressionNode()
	{
		ArrayAccessExpress *arrayExp = new ArrayAccessExpress(mOperatorEntry);
		arrayExp->GetIndexExpression() = mAccessPosExpression;
		mAccessPosExpression = NULL;
		return arrayExp;
	}
};

struct DoCallOperator : public Operator
{
	// 实参表达式列表
	std::list<PostfixExpression*> mRealParams;
	
	int Compile(SimpleCScriptEngContext *context)
	{
		int r = -1;
		char c;
		std::list<PostfixExpression*> paramCallList;
		while (1)
		{
			PostfixExpression *pc = new PostfixExpression;
			if ((r = context->ParseExpressionEndWith(c, pc, ",)")) != 0)
			{
				break;
			}

			if (c == ')')
			{
				// 如果没有一个参数，则删除最后一个
				if (pc->isEmpty())
				{
					delete pc;
				}
				else
				{
					paramCallList.push_back(pc);
				}
				r = 0;
				break;
			}
			paramCallList.push_back(pc);
		}
		if (r < 0)
		{
			std::for_each(paramCallList.begin(), paramCallList.end(),
				[](PostfixExpression *p)
			{
				delete p;
			});
		}
		else
		{
			mRealParams = paramCallList;
		}
		return r;
	}

	virtual ExpressionNode* CreateExpressionNode()
	{
		SubProcCallExpression *subCall = new SubProcCallExpression(mOperatorEntry);
		subCall->GetRealParams() = mRealParams;
		mRealParams.clear();
		return subCall;
	}
};

///////////////////////////////////////////////////////////////////////////////

void SimpleCScriptEngContext::OnOperator(PostfixExpression *pe,
	std::list<Operator*> *operList, 
	bool &lastIsOperatorOrFirstInLocal,
	Operator *oper)
{
	lastIsOperatorOrFirstInLocal = true;
	while (1)
	{
		if (operList->empty()
			|| operList->back()->mOperatorEntry->priority < oper->mOperatorEntry->priority
			// 处理右结合的问题
			|| (oper->mOperatorEntry->combineDir && operList->back()->mOperatorEntry->priority == oper->mOperatorEntry->priority))
		{
			operList->push_back(oper);
			break;
		}
		else
		{
			pe->AddNode(operList->back()->CreateExpressionNode());
			delete operList->back();
			operList->pop_back();
		}
	}
}

int SimpleCScriptEngContext::ParseExpressionEndWith(char &c, 
	PostfixExpression *pe, const std::string &terminalC)
{
	int r, parseResult;
	Symbol symbol;
	char endC;

	std::list<Operator*> operatorList;

	// 表示是否是当前（开始后的第一个，或者之前的那个符号是不是运算符
	bool lastIsOperatorOrFirstInLocal = true;
	std::string lastSymbol;
	while ((r = GetNextSymbol(symbol)) == 0)
	{
		if (symbol.type != Symbol::String && symbol.symbolOrig.size() == 1 && terminalC.find(symbol.symbolOrig[0]) != terminalC.npos)
		{
			c = symbol.symbolOrig[0];
			break;
		}

		switch (symbol.type)
		{
		case Symbol::CommonSymbol:
		case Symbol::constInt:
		case Symbol::constFloat:
		case Symbol::String:
		case Symbol::SingleChar:
			lastIsOperatorOrFirstInLocal = false;
			lastSymbol = symbol.symbolOrig;
			pe->AddNode(new SymbolExpressionNode(symbol));
			continue;

		default:
			break;
		}

		if (symbol.symbolOrig[0] == ';')
		{
			// 如果结束字符中，没有说明';'是结束字符，但是却读到了该字符
			// 说明解析存在错误
			return -1;
		}

		if (symbol.symbolOrig[0] == '(')
		{
			if (lastIsOperatorOrFirstInLocal)
			{
				parseResult = ParseExpressionEndWith(endC, pe, ")");
				if (parseResult < 0)
					return parseResult;
				lastIsOperatorOrFirstInLocal = false;
			}
			else
			{
				// 函数调用
				DoCallOperator *doCallOperator = new DoCallOperator;
				doCallOperator->mOperatorEntry = OperatorHelper::GetOperatorEntryByOperatorString("docall");
				if ((parseResult = doCallOperator->Compile(this)) < 0)
					return parseResult;
				OnOperator(pe, &operatorList, lastIsOperatorOrFirstInLocal, doCallOperator);
				lastIsOperatorOrFirstInLocal = false;
			}
		}
		else if (symbol.symbolOrig[0] == '[')
		{
			if (lastIsOperatorOrFirstInLocal)
				return -2;
			IndexAccessOperator *indexOperator = new IndexAccessOperator;
			indexOperator->mOperatorEntry = OperatorHelper::GetOperatorEntryByOperatorString("getbyindex");
			if ((parseResult = indexOperator->Compile(this)) < 0)
				return parseResult;
			OnOperator(pe, &operatorList, lastIsOperatorOrFirstInLocal, indexOperator);
		}
		else
		{
			const OperatorHelper::OperatorTableEntry *entry = OperatorHelper::GetOperatorEntryByOperatorString(symbol.symbolOrig.c_str());
			if (entry)
			{
				if (lastIsOperatorOrFirstInLocal || pe->isEmpty())
				{
					if (entry->operatorId == OP_bitwise_and)
						entry = OperatorHelper::GetOperatorEntryByOperatorString("getaddressof&");
					//else if (entry->operatorId == OP_mul)
					//	entry = OperatorHelper::GetOperatorEntryByOperatorString("getrefof*");
					else if (entry->operatorId == OP_sub)
						entry = OperatorHelper::GetOperatorEntryByOperatorString("neg-");
				}
				Operator *oper = new CommonOperator;
				oper->mOperatorEntry = entry;

				OnOperator(pe, &operatorList, lastIsOperatorOrFirstInLocal, oper);
			}
			else
			{
				lastIsOperatorOrFirstInLocal = false;
				lastSymbol = symbol.symbolOrig;
				pe->AddNode(new SymbolExpressionNode(symbol));
			}
		}
	}

	if (r > 0)
		c = '\0';

	while (!operatorList.empty())
	{
		pe->AddNode(operatorList.back()->CreateExpressionNode());
		delete operatorList.back();
		operatorList.pop_back();
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
