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

	// ��¼�ؼ�����ת����Ϊ�˿���ȷ�������Ƿ�Ϊ�ؼ���
	// ����ռ�õ��ڴ�Ƚϴ󣬵���Ϊ�˷��㴦����������ѹ��
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
			CK_STRING, // ��չ���ͣ�ֱ�ӿ��Զ����ַ�������
			//CK_UNSIGNED,// Ϊ�˷�����룬��ȥ����ؼ���
			// ��������
			CK_ARRAY,
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

			// ���䣬�����ڴ����й�����debugָ��
			CK_DEBUGBREAK,
		};

		struct KeywordsEntry
		{
			const char *keywords;
			int id;
		};

		struct TransNode
		{
			// ��ʾ��ǰ�ڵ��Ƿ��ʾһ���ؼ���
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
			return keywordType >= CK_CHAR && keywordType <= CK_ARRAY;
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
			// ���ڼ���������������ʽ������switch��䣩
			bool (*CalcIntConst)(int &a, int l, int r);
			// ָ����Ϸ���0�����ϣ�1���ҽ��
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
		virtual void DebugPrint() const = 0;
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
		virtual void DebugPrint() const;
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);

		Symbol& GetSymbol() { return mSymbol; }
	};

	class OperatorExpressionNode : public ExpressionNode
	{
		friend class SymbolExpressionNode;
		friend class PostfixExpression;
		DECLARE_OBJINFO(OperatorExpressionNode)

	protected:
		// ����ExpressionNode�ڽ���Ϊ�﷨��ʱ����ʹ�ã���������������ʽ
		// mHasConstruct������ǰ�ǲ����Ѿ�����Ϊ���﷨���������﷨��
		// �����ӱ���ʽҲ����Ч��
		bool mHasConstruct;

		// ���������ڴ�������2Ԫ������ʱ��Ҳ�Ὣ�䴦��Ϊ2Ԫ
		// �����������ָʾ�Ƿ��Ѿ������������ֱ���ʽ������еĻ���
		bool mHasGenerate;

		// ָʾ������ʱ���Ƿ�ɾ���Ӷ���
		bool mDoNotDeleteSubOnDestory;

		const OperatorHelper::OperatorTableEntry *mOperatorEntry;
		ExpressionNode *mSubNodes[2];

		//// �����.�����������ڷ������ŵ�ʱ�򣬻Ὣ�ó�Ա����Ϊ�ַ�����id
		//uint32_t mDotOperatorStrId;

		// ���ǵ������ȡ��Ա��������ʱ����¼��Ա������
		std::string mMemberName;

	public:
		OperatorExpressionNode(const OperatorHelper::OperatorTableEntry *operatorEntry);
		virtual ~OperatorExpressionNode();
		virtual void DebugPrint() const;
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);
	};

	// �ӹ��̵��õı���ʽ
	class SubProcCallExpression : public OperatorExpressionNode
	{
		DECLARE_OBJINFO(SubProcCallExpression)

	private:
		//std::string mSubProcName;
		// ʵ�α���ʽ�б�
		std::list<PostfixExpression*> mRealParams;

	public:
		SubProcCallExpression(const OperatorHelper::OperatorTableEntry *operatorEntry);
		virtual ~SubProcCallExpression();
		virtual void DebugPrint() const;
		virtual void BeforeAddToPostfixExpression(PostfixExpression *postfixExpression);
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);

		std::list<PostfixExpression*>& GetRealParams() { return mRealParams; }
	};

	// '?'����ʽ��һ����Ԫ����ʽ����������Ķ�Ԫ����ʽת��Ϊ������ʽ
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

	// �������ã���������ֵҲ��������ֵ�����﷨�������ڴ�����
	class ArrayAccessExpress : public OperatorExpressionNode
	{
		DECLARE_OBJINFO(ArrayAccessExpress)

	private:
		// ָʾ��������������ʽ
		PostfixExpression *mAccessPosExpression;

	public:
		ArrayAccessExpress(const OperatorHelper::OperatorTableEntry *operatorEntry);
		virtual ~ArrayAccessExpress();
		virtual void DebugPrint() const;
		virtual void BeforeAddToPostfixExpression(PostfixExpression *postfixExpression);
		virtual int GenerateInstruction(Statement *statement, CompileResult *compileResult);

		PostfixExpression*& GetIndexExpression() { return mAccessPosExpression; }
	};

	class PostfixExpression
	{
	private:
		// ��׺����ʽ
		std::list<ExpressionNode*> mPostfixExpression;

		// ��������󣬻Ὣ�﷨���ŵ�����
		ExpressionNode *mGrammaTreeRoot;

	private:
		bool isGetMember(OperatorExpressionNode *n, ExpressionNode *left, ExpressionNode *right) const;

		static bool HasChildren(ExpressionNode *node)
		{
			return node->isInheritFrom(OBJECT_INFO(OperatorExpressionNode));
		}

		bool CalcNode(ExpressionNode *n, int &val);
		void OnRemoveSingleGrammaTreeNode(ExpressionNode *n, std::list<ExpressionNode*> &l);

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

		void DebugPrint() const;

		bool IsConstIntergerExpression(int &val);
	};

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
		static uint32_t BlockDistance(StatementType *parent, Statement *sun)
		{
			uint32_t c = 0;
			Statement *s = sun->GetParent();
			while (s)
			{
				if (static_cast<StatementType*>(static_cast<Statement*>(s)) == parent)
					break;

				if (s->isInheritFrom(OBJECT_INFO(StatementBlock)))
					c++;

				s = s->GetParent();
			}
			return c;
		}

		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context) { return -1; }
		virtual int GenerateInstruction(CompileResult *compileResult) {
#if defined(DEBUG) || defined(_DEBUG)
			SCRIPT_TRACE("GenerateInstruction function does not implement by [%s].\n",
				GetThisObjInfo()->className);
#endif
			return -1;
		}
		const Statement* GetParent() const { return mParentBlock; }
		Statement* GetParent() { return mParentBlock; }
		StatementBlock* GetBlockParent();
		const StatementBlock* GetBlockParent() const;

		Statement* isInLoopStatementBlock(uint32_t breakOrContinue);
		virtual uint32_t isLoopStatement() const { return 0; }
		virtual int GenerateBreakStatementCode(BreakStatement *bs, CompileResult *compileResult) { return -1; }
		virtual int GenerateContinueStatementCode(ContinueStatement *cs, CompileResult *compileResult) { return -1; }
	};

	// ���ɱ���ʽ���ɵ����
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
		bool mSaveFrame;

		// ���浱ǰ������ж�������֣���������
		std::map<std::string,uint32_t> mLocalNameStack;
		// �����0����ʾ��ͨ�Ķ������ֶ���������ⲿ�������ִ��ǰ
		// ѹ���ջ������Ӧ�ô��������ı�������
		std::vector<uint32_t> mLocalNameType;

		std::list<Statement*> mStatementList;

	private:
		//// ������������ת��create***ָ��
		//runtime::CommonInstruction DeclTypeToCreateInstuction(uint32_t declType) const;
		// �����������ͣ�����create***ָ��
		void InsertCreateTypeInstructionByDeclType(uint32_t declType, GenerateInstructionHelper *giHelper);

	public:
		StatementBlock();
		virtual ~StatementBlock();

		void SetSaveFrame(bool sf) { mSaveFrame = sf; }
		void AddStatement(Statement *statement);
		virtual int Compile(Statement *parent, SimpleCScriptEngContext *context);
		int Compile(Statement *parent, SimpleCScriptEngContext *context, bool mayLackOfBrace);
		int PushName(const char *name, uint32_t declType = 0);
		// �ӵ�ǰ����鿪ʼѰ�ҷ��Ŷ��壬���û���ҵ�����false�������level��index����λ��
		bool FindName(const char *name, uint32_t &level, uint32_t &index) const;
		virtual int GenerateInstruction(CompileResult *compileResult);
		int CompileWithCertainParent(Statement *parent, SimpleCScriptEngContext *context, bool mayLackOfBrace);
		std::list<Statement*>& GetStatementList() { return mStatementList; }
	};

	// TODO: ע�⣬��������У�û�д���ָ�루�����༶ָ�룩��Ҳû�д�������������������ʼ����
	class DeclareStatement : public Statement
	{
		DECLARE_OBJINFO(DeclareStatement)
		friend class StatementBlock;

	private:
		// int,float��
		int mDeclType;
		std::string mTypeString;

		struct DeclareInfo
		{
			std::string mVarName;
			// ����ֵ�ı���ʽ������еĻ���
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
		// break��continue���Ŀ�λ����ѭ��������ɴ������ǰ����Щλ�ñ���
		// ����������ת�����д
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

	class SwitchStatement
		: public Statement
		, public BreakContinueSupport
	{
		DECLARE_OBJINFO(SwitchStatement)

	private:
		PostfixExpression mSwitchExpression;

		// ��¼��ÿ����֧
		struct SwitchCaseEntry
		{
			// ȱʡ��case��
			bool mIsDefaultSection;

			// ��������ʽ
			int mIntergerConst;

			// ���ں��������б�
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
		// ������break��continue
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
		// �ս��ַ�����
		static const char mTerminalAlpha[];
		// ������Ը��������ս��ַ�����
		static const char mDupTermAlpha[];
		// ������Ը�=���ս��ַ�����
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
		StatementBlock mTopLevelBlock;

	private:
		// ����>=256��ֵ��ʾ�������ս��ַ�terminal
		// ����>=0��ֵ��ʾʵ�ʵ�ASCII�ַ�
		// ����-1��ʾ���ִ��󣨱���ת���ַ����ԣ��������˻س����з�
		int GetNextSingleChar(char terminal);
		bool isBlank(char c);
		bool isTerminalAlpha(char c) const;
		bool mayFollowWithEqual(char c) const;

		// ����GetNextSymbol���Ի���
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
		void GoBack();
		int ParseExpressionEndWith(char &c, PostfixExpression *pe, const std::string &terminalC = "\0");
		int PushName(const char *name);

		CompileResult& GetCompileResult() { return *mCompileResult; }

		HANDLE Compile(scriptAPI::ScriptSourceCodeStream *codeStream, bool end = false);
		
		SimpleCScriptEngContext();
		~SimpleCScriptEngContext();
	};
}