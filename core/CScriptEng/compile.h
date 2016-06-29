#pragma once
#include "CScriptEng.h"
#include "scriptDef.h"

#define RETHELP(result) return ((result) > 0 ? -1 : (result))

namespace compiler
{
	class SimpleCScriptEng;
	class StatementBlock;
	class Statement;
	class BreakStatement;
	class ContinueStatement;

	// 记录关键字跳转表，为了快速确定符号是否为关键字
	// 这里占用的内存比较大，但是为了方便处理，不考虑压缩
	class KeywordsTransTable
	{
	public:
		enum
		{
			CK_NULL,
			CK_CHAR,
			CK_UCHAR,
			CK_SHORT,
			CK_USHORT,
			CK_INT,
			CK_UINT,
			CK_FLOAT,
			CK_DOUBLE,
			CK_STRING, // 扩展类型，直接可以定义字符串类型
			//CK_UNSIGNED,// 为了方便编译，略去这个关键字
			// 扩充类型
			CK_ARRAY,
			// 扩充类型
			CK_OBJECT,
			// 扩充，函数调用
			CK_FUNCTION,

			CK_VOID,
			CK_IF,
			CK_ELSE,
			CK_WHILE,
			CK_DO,
			CK_BREAK,
			CK_CONTINUE,
			CK_FOR,
			CK_GOTO,
			CK_SWITCH,
			CK_CASE,
			CK_DEFAULT,
			CK_STRUCT,
			CK_RETURN,

			// 扩充，用来在代码中插入debug指令
			CK_DEBUGBREAK,
		};

		struct KeywordsEntry
		{
			const char *keywords;
			int id;
		};

		struct TransNode
		{
			// 表示当前节点是否表示一个关键字
			uint16_t keywordsId;
			char reserved;
			char theChar;
			TransNode *theNext[256];

			TransNode(char c);
			~TransNode();
		};

	private:
		TransNode *mCurrent;
		TransNode *mRoot;

	public:
		const static KeywordsEntry mKeywords[];
		const static uint32_t mKeywordsCount;

		void Init();
		void Term();

		KeywordsTransTable();
		~KeywordsTransTable();

		const TransNode* GoNext(char c);
		void Reset();

		static bool isDataType(int keywordType) {
			return keywordType >= CK_CHAR && keywordType <= CK_FUNCTION;
		}
	};

	enum
	{
		OP_mod,
		OP_logic_and,
		OP_logic_or,
		OP_logic_not,
		OP_bitwise_xor,
		OP_bitwise_and,
		OP_bitwise_or,
		OP_bitwise_not,
		OP_mul,
		OP_sub,
		OP_add,
		OP_div,
		OP_setval,
		OP_lessthan,
		OP_greaterthan,

		OP_isnotequal,
		OP_mod_setval,
		OP_bitwise_xor_setval,
		OP_bitwise_and_setval,
		OP_mul_setval,
		OP_add_setval,
		OP_sub_setval,
		OP_div_setval,
		OP_isequal,
		OP_bitwise_or_setval,
		OP_lessthan_equalto,
		OP_greaterthan_equalto,

		OP_and,
		OP_or,

		OP_questionmark,
		OP_colon,

		OP_commas,
		OP_fullstop,
		OP_dot = OP_fullstop,

		OP_getaddress,
		OP_getrefof,

		OP_neg,
		OP_docall,
		OP_getbyindex,

		OP_getmember,
	};

	class OperatorHelper
	{
	public:
		struct OperatorTableEntry
		{
			int operatorId;
			const char *operString;
			int participator;
			int priority;
			// 用于计算整数常量表达式（用于switch语句）
			bool (*CalcIntConst)(int &a, int l, int r);
			// 指定结合方向，0是左结合，1是右结合
			int combineDir;
		};

	private:
		static const OperatorTableEntry mOperTable[];
		static std::map<std::string, const OperatorTableEntry*> mOperatorEntryTable;

	public:
		static void InitEntryTable();
		static void TermEntryTable();

		static const OperatorTableEntry* GetOperatorEntryByOperatorString(const char *s);
	};

	class SimpleCScriptEngContext;
	class PostfixExpression;

	class ExpressionNode : public objBase
	{
		friend class PostfixExpression;
		DECLARE_OBJINFO(ExpressionNode)

	protected:
		ExpressionNode *mParent;

	public:
		ExpressionNode();
		virtual ~ExpressionNode();
		ExpressionNode* GetParent() const { return mParent; }
		virtual void BeforeAddToPostfixExpression(PostfixExpression *postfixExpression);
		virtual void AfterAddToPostfixExpression(PostfixExpression *postfixExpression);
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult) {
#if defined(DEBUG) || defined(_DEBUG)
			SCRIPT_TRACE("expression node [%s] has not implement the GenerateInstruction function.\n",
				GetThisObjInfo()->className);
#endif
			return -12;
		}
	};

	struct CodePosition
	{
		uint32_t column;
		uint32_t line;
	};

	struct Symbol
	{
		enum {
			CommonSymbol,
			Keywords,
			constInt,
			constFloat,
			String,
			SingleChar,
			Array,
			terminalChar,
		};

		int type;
		int keywordsType;
		std::string symbolOrig;
		CodePosition codePos;

		Symbol();
	};

	class SymbolExpressionNode : public ExpressionNode
	{
		DECLARE_OBJINFO(SymbolExpressionNode)

	private:
		Symbol mSymbol;

	public:
		SymbolExpressionNode(const Symbol &symbol);
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);

		Symbol& GetSymbol() { return mSymbol; }
	};

	class OperatorExpressionNode : public ExpressionNode
	{
		friend class SymbolExpressionNode;
		friend class PostfixExpression;
		DECLARE_OBJINFO(OperatorExpressionNode)

	protected:
		// 由于ExpressionNode在解析为语法树时还会使用，而对于算数表达式
		// mHasConstruct表明当前是不是已经解析为了语法树，对于语法树
		// 各种子表达式也是有效的
		bool mHasConstruct;

		// 本编译器在处理超过2元的运算时，也会将其处理为2元
		// 这个变量用来指示是否已经处理了左右子表达式（如果有的话）
		bool mHasGenerate;

		// 指示析构的时候是否删除子对象
		bool mDoNotDeleteSubOnDestory;

		const OperatorHelper::OperatorTableEntry *mOperatorEntry;
		ExpressionNode *mSubNodes[2];
		
		// 当是点操作（取成员操作符）时，记录成员的名称
		std::string mMemberName;

	public:
		OperatorExpressionNode(const OperatorHelper::OperatorTableEntry *operatorEntry);
		virtual ~OperatorExpressionNode();
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);
	};

	// 子过程调用的表达式
	class SubProcCallExpression : public OperatorExpressionNode
	{
		DECLARE_OBJINFO(SubProcCallExpression)

	private:
		// 实参表达式列表
		std::list<PostfixExpression*> mRealParams;

	public:
		SubProcCallExpression(const OperatorHelper::OperatorTableEntry *operatorEntry);
		virtual ~SubProcCallExpression();
		virtual void BeforeAddToPostfixExpression(PostfixExpression *postfixExpression);
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);

		std::list<PostfixExpression*>& GetRealParams() { return mRealParams; }
	};

	// '?'表达式（一个三元表达式），从虚拟的二元表达式转换为本表达式
	class QuestionExpression : public OperatorExpressionNode
	{
		friend class PostfixExpression;
		DECLARE_OBJINFO(QuestionExpression)

	private:
		ExpressionNode *mJudgement;
		ExpressionNode *mTrueExpression;
		ExpressionNode *mFalseExpression;

	public:
		QuestionExpression(const OperatorHelper::OperatorTableEntry *operatorEntry);
		virtual ~QuestionExpression();
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);
	};

	// 数组引用（可能是左值也可能是右值，由语法分析后期处理）
	class ArrayAccessExpress : public OperatorExpressionNode
	{
		DECLARE_OBJINFO(ArrayAccessExpress)

	private:
		// 指示索引的算数表达式
		PostfixExpression *mAccessPosExpression;

	public:
		ArrayAccessExpress(const OperatorHelper::OperatorTableEntry *operatorEntry);
		virtual ~ArrayAccessExpress();
		virtual void BeforeAddToPostfixExpression(PostfixExpression *postfixExpression);
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);

		PostfixExpression*& GetIndexExpression() { return mAccessPosExpression; }
	};

	class FunctionStatement;
	class FunctionDefinationExpress : public ExpressionNode
	{
		DECLARE_OBJINFO(FunctionDefinationExpress)

	private:
		std::string mName;
		FunctionStatement *mFuncStatement;

	public:
		FunctionDefinationExpress(const std::string &funcName);
		virtual ~FunctionDefinationExpress();
		int Compile(SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(Statement *parent, CompileResult *compileResult);
	};

	class PostfixExpression
	{
	private:
		// 后缀表达式
		std::list<ExpressionNode*> mPostfixExpression;

		// 经过编译后，会将语法树放到这里
		ExpressionNode *mGrammaTreeRoot;

	private:
		bool isGetMember(OperatorExpressionNode *n, ExpressionNode *left, ExpressionNode *right) const;

		static bool HasChildren(ExpressionNode *node)
		{
			return node->isInheritFrom(OBJECT_INFO(OperatorExpressionNode));
		}

		bool CalcNode(ExpressionNode *n, int &val);
		void OnRemoveSingleGrammaTreeNode(ExpressionNode *n, std::list<ExpressionNode*> &l);

		void ThrowBadcast(const char *s);

	public:
		PostfixExpression();
		~PostfixExpression();
		void Clear();
		void RemoveGrammaTree(ExpressionNode *root);

		ExpressionNode* GetTreeRoot() { return mGrammaTreeRoot; }

		bool isEmpty() const { return mPostfixExpression.empty(); }
		void RemoveTail();
		void AddNode(ExpressionNode *subNode);
		int CreateExpressionTree();
		int GenerateInstruction(Statement *statement, CompileResult *compileResult);
		static int GenerateInstruction(ExpressionNode *root, Statement *statement, CompileResult *compileResult);

		bool IsConstIntergerExpression(int &val);
	};

	class FunctionStatement;
	class Statement : public objBase
	{
		friend class StatementBlock;
		DECLARE_OBJINFO(Statement)

	protected:
		Statement *mParentBlock;

	public:
		enum
		{
			SupportBreak = 1 << 0,
			SupportContinue = 1 << 1,
		};

	public:
		Statement();
		virtual ~Statement();

		template <typename StatementType>
		static bool BlockDistance(StatementType *parent, Statement *sun, uint32_t &dist);

		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context) { return -1; }
		virtual int GenerateInstruction(CompileResult *compileResult) {
			SCRIPT_TRACE("GenerateInstruction function does not implement by [%s].\n",
				GetThisObjInfo()->className);
			return -1;
		}
		const Statement* GetParent() const { return mParentBlock; }
		Statement* GetParent() { return mParentBlock; }

		FunctionStatement* GetFunctionParent();
		const FunctionStatement* GetFunctionParent() const;
		
		// 由于StatementBlock和FunctionStatement都是变量的容器
		// 所以，这里是找到最近的Block或者Function，并注册变量
		bool RegistName(const char *name, uint32_t type);
		bool FindName(const char *name, uint32_t &l, uint32_t &i) const;

		// 上面的FindName要配合这个函数来判断返回的数据是否正确，
		// 由于上面的FindName仅返回name距离当前Function的距离，但是目前本语言不支持
		// 闭包，所以不允许将上一层的Function中的数据自动传到本级数据，除了顶级函数之外
		// 又由于FindName是递归实现的，所以这里只能由调用FindName的地方来检查一下了
		bool CheckValidLayer(uint32_t l) const;

		Statement* isInLoopStatementBlock(uint32_t breakOrContinue);
		virtual uint32_t isLoopStatement() const { return 0; }
		virtual int GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult) { return -1; }
		virtual int GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult) { return -1; }
	};

	// 仅由表达式构成的语句
	class PureExpressionStatement : public Statement
	{
		DECLARE_OBJINFO(PureExpressionStatement)

	private:
		PostfixExpression mExp;

	public:
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	class StatementBlock : public Statement
	{
		DECLARE_OBJINFO(StatementBlock)

	protected:
		// key中保存当前块中保存的变量名列表
		// value中保存该变量在父级VariableContainer中的id
		std::map<std::string,size_t> mLocalNameStack;
		std::vector<uint32_t> mPointerToType;

		std::list<Statement*> mStatementList;

	public:
		StatementBlock();
		virtual ~StatementBlock();

		void AddStatement(Statement *statement);
		int Compile(Statement *parent, SimpleCScriptEngContext *context, bool beginWithBrace);

		bool RegistNameInBlock(const char *name, uint32_t declType = 0);
		bool FindNameInBlock(const char *name, uint32_t &l, uint32_t &i) const;
		std::list<Statement*>& GetStatementList() { return mStatementList; }

		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	// TODO: 注意，声明语句中，没有处理指针（包括多级指针），也没有处理数组声明（包括初始化）
	class DeclareStatement : public Statement
	{
		DECLARE_OBJINFO(DeclareStatement)
		friend class StatementBlock;

	private:
		// int,float等
		int mDeclType;
		std::string mTypeString;

		struct DeclareInfo
		{
			std::string mVarName;
			// 赋初值的表达式（如果有的话）
			PostfixExpression *mSetValueExpression;
		};
		std::list<DeclareInfo> mInfoList;

	public:
		DeclareStatement();
		virtual ~DeclareStatement();
		
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	class IfConditionStatement : public Statement
	{
		DECLARE_OBJINFO(IfConditionStatement)

		struct ElseIfBlock
		{
			PostfixExpression *judg;
			StatementBlock *sb;
		};

	private:
		PostfixExpression mJudgementStatement;
		StatementBlock mTrueBlock;
		std::list<ElseIfBlock> mElseIfStatementList;
		StatementBlock *mElseStatement;

	public:
		IfConditionStatement();
		virtual ~IfConditionStatement();
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	class BreakContinueSupport
	{
	protected:
		// break和continue语句的坑位，在循环语句生成代码结束前，这些位置必须
		// 被真正的跳转语句重写
		std::list<uint32_t> mBreakToFillList;
		std::list<uint32_t> mContinueToFillList;

	public:
		int FillBreakList(CompileResult *compileResult, uint32_t actureJumpPos);
		int FillContinueList(CompileResult *compileResult, uint32_t actureJumpPos);
	};

	class ForStatement : public Statement
		, public BreakContinueSupport
	{
		DECLARE_OBJINFO(ForStatement)

	private:
		PostfixExpression *mDeclExpression;
		PostfixExpression *mJudgementExpression;
		PostfixExpression *mIteratorExpression;
		StatementBlock mStatementBlock;

	public:
		ForStatement();
		virtual ~ForStatement();
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual uint32_t isLoopStatement() const { return SupportContinue | SupportBreak; }
		virtual int GenerateInstruction(CompileResult *compileResult);
		virtual int GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult);
		virtual int GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult);
	};

	class DoWhileStatement : public Statement
		, public BreakContinueSupport
	{
		DECLARE_OBJINFO(DoWhileStatement)

	protected:
		PostfixExpression mJudgementExpression;
		StatementBlock mStatementBlock;

	public:
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual uint32_t isLoopStatement() const { return SupportContinue | SupportBreak; }
		virtual int GenerateInstruction(CompileResult *compileResult);
		virtual int GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult);
		virtual int GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult);
	};

	class WhileStatement : public DoWhileStatement
	{
		DECLARE_OBJINFO(WhileStatement)

	public:
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual uint32_t isLoopStatement() const { return SupportContinue | SupportBreak; }
		virtual int GenerateInstruction(CompileResult *compileResult);
		virtual int GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult);
		virtual int GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult);
	};

	class BreakStatement : public Statement
	{
		DECLARE_OBJINFO(BreakStatement)

	public:
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	class ContinueStatement : public Statement
	{
		DECLARE_OBJINFO(ContinueStatement)

	public:
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	class DebugBreakStatement : public Statement
	{
		DECLARE_OBJINFO(DebugBreakStatement)

	public:
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	class FunctionStatement
		: public Statement
	{
		friend class FunctionDefinationExpress;
		DECLARE_OBJINFO(FunctionStatement)

	public:
		static const char mEntryFuncName[];

	protected:
		// 当前层容纳的变量
		std::map<std::string,uint32_t> mLocalName;
		std::vector<uint32_t> mLocalType;
		bool mIsTopLevel;

	private:
		// 是否是在表达式中的匿名函数
		bool mIsAnonymousFuncInExpression;
		std::string mName;

		struct Param
		{
			std::string name;
		};
		std::list<Param> mParamList;

		StatementBlock mFunctionBody;
		
	private:
		int ParseParamList(SimpleCScriptEngContext *context);
		int GenerateLocalVariableInstruction(CompileResult *compileResult);
				
		//// 由声明的类型转成create***指令
		//runtime::CommonInstruction DeclTypeToCreateInstuction(uint32_t declType) const;
		// 根据声明类型，插入create***指令
		void InsertCreateTypeInstructionByDeclType(uint32_t declType, GenerateInstructionHelper *giHelper);

		int GenerateNamedFunctionCode(CompileResult *compileResult);
		int GenerateAnonymousFunctionCode(CompileResult *compileResult);

	public:
		FunctionStatement();
		FunctionStatement(const std::string &name);
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
		std::string GetName() const { return mName; }
		bool isTopLevelFun() const { return mIsTopLevel; }

		FunctionStatement* GetParentFunction();
		
		bool RegistNameInContainer(const char *name, uint32_t declType);
		bool FindNameInContainer(const char *name, uint32_t &level, uint32_t &index) const;
		bool GetNameType(uint32_t id, uint32_t &type) const {
			if (id >= mLocalType.size())
				return false;
			type = mLocalType[id];
			return true;
		}
		bool SetNameType(uint32_t id, uint32_t type) {
			if (id >= mLocalType.size())
				return false;
			mLocalType[id] = type;
			return true;
		}
		uint32_t ReservedVariableRoom(uint32_t type = -1);
	};

	class ReturnStatement : public Statement
	{
		DECLARE_OBJINFO(ReturnStatement)

		PostfixExpression mExp;

	public:
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		virtual int GenerateInstruction(CompileResult *compileResult);
	};

	class SwitchStatement
		: public Statement
		, public BreakContinueSupport
	{
		DECLARE_OBJINFO(SwitchStatement)

	private:
		PostfixExpression mSwitchExpression;

		// 记录了每个分支
		struct SwitchCaseEntry
		{
			// 缺省是case块
			bool mIsDefaultSection;

			// 常量表达式
			int mIntergerConst;

			// 接在后面的语句列表
			std::list<StatementBlock*> mStatementBlocks;

			uint32_t beginPos;

			SwitchCaseEntry();
		};
		std::list<SwitchCaseEntry*> mCaseList;

		std::set<int> mCaseValueSet;

		void ClearSwitchCaseEntry(SwitchCaseEntry *entry);
		int OnCaseOrDefault(SimpleCScriptEngContext *context, bool isDefault = false);
		bool HasSameCaseValue(int val) const {
			return mCaseValueSet.find(val) != mCaseValueSet.end();
		}

	public:
		SwitchStatement();
		~SwitchStatement();
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		// 可以用break和continue
		virtual uint32_t isLoopStatement() const { return SupportBreak; }
		virtual int GenerateInstruction(CompileResult *compileResult);
		virtual int GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult);
	};

	struct Operator
	{
		const OperatorHelper::OperatorTableEntry *mOperatorEntry;
		virtual ExpressionNode* CreateExpressionNode() = 0;

		virtual ~Operator();
	};

	class SimpleCScriptEngContext
	{
		friend class SimpleCScriptEng;
		friend class SubProcCallExpression;
		friend class ArrayAccessExpress;

	private:
		// 终结字符集合
		static const char mTerminalAlpha[];
		// 后面可以跟本身的终结字符集合
		static const char mDupTermAlpha[];
		// 后面可以跟=的终结字符集合
		static const char mFollowWithEqual[];

	private:
		const char *mParseCurrent;
		const char *mEnd;
		CodePosition mCodePosition;
		static std::set<char> mTerminalAlphaSet;
		static std::set<char> mMayFollowWithEqualSet;

		scriptAPI::ScriptSourceCodeStream *mCodeStream;
		std::string mSourceCode;

		int mState;

		KeywordsTransTable mKeywordsTransTable;

		ConstStringData mConstStringData;
		CompileResult *mCompileResult;
		FunctionStatement mTopLevelFunction;

	private:
		// 返回>=256的值表示读到了终结字符terminal
		// 返回>=0的值表示实际的ASCII字符
		// 返回-1表示出现错误（比如转意字符不对）、读到了回车换行符
		int GetNextSingleChar(char terminal);
		bool isBlank(char c);
		bool isTerminalAlpha(char c) const;
		bool mayFollowWithEqual(char c) const;

		// 允许GetNextSymbol可以回退
		static const int mSymbolStackMaxSize = 10;
		std::list<Symbol> mSymbolStack;
		std::list<Symbol>::iterator mCurrentStack;
		
		void OnOperator(PostfixExpression *pe, 
			std::list<Operator*> *operList, 
			bool &lastIsOperatorOrFirstInLocal, Operator *oper);

	public:
		static void Init();
		static void Term();

		int BeginParseSymbol(scriptAPI::ScriptSourceCodeStream *codeStream);
		int GetNextSymbol(Symbol &symbol);
		bool GetNextSymbolMustBe(Symbol &symbol, const std::string &v);
		void GoBack();
		int ParseExpressionEndWith(char &c, PostfixExpression *pe, const std::string &terminalC = "\0");
		int PushName(const char *name);
		int FindGlobalName(const char *name);

		CompileResult& GetCompileResult() { return *mCompileResult; }

		HANDLE Compile(scriptAPI::ScriptSourceCodeStream *codeStream, bool end = false);
		
		SimpleCScriptEngContext();
		~SimpleCScriptEngContext();
	};

	template <typename StatementType>
	bool Statement::BlockDistance(StatementType *parent, Statement *sun, uint32_t &dist)
	{
		dist = 0;
		Statement *s = sun->GetParent();
		while (s)
		{
			if (s->isInheritFrom(OBJECT_INFO(StatementType)) && static_cast<StatementType*>(s) == parent)
				break;

			if (s->isInheritFrom(OBJECT_INFO(FunctionStatement)))
				return false;

			if (s->isInheritFrom(OBJECT_INFO(StatementBlock)))
				dist++;

			s = s->GetParent();
		}
		return true;
	}
}
