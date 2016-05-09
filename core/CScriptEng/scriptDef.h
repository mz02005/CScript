#pragma once
#include "notstd/notstd.h"

namespace runtime
{
	class runtimeContext;
	class FunctionObject;

	enum
	{
		// 将栈顶的2个元素弹出，并进行计算，将计算得到的结果压入堆栈
		VM_add = 1,
		VM_sub,
		VM_mul,
		VM_div,
		VM_mod,
		VM_logicAnd,
		VM_logicOr,
		VM_logicNot,
		VM_bitwiseAnd,
		VM_bitwiseOr,
		VM_bitwiseXOR,
		VM_bitwiseNot,
		VM_setVal,

		// 都是对栈上的两个元素进行比较的指令，同时将这两个元素出栈
		// 如果比较结果满足条件，则在栈中压入整数1，否则压入整数0
		VM_equal,
		VM_notEqual,
		VM_greater,
		VM_less,
		VM_greaterEqual,
		VM_lessEqual,
		// 测试栈顶元素是否为0，如果为0，则跳转到指定的位置执行
		// 该指令需要额外的4字节数据，用于标志跳转的位置
		// 对于所有的数值类型，该指令总是判定该值是否为0，对于其它的类型
		// 该函数总是不跳转（即总是认为其值不为0）
		VM_jz,
		// 无条件跳转到指定位置，该指令也需要额外的4个字节用于指示目标位置
		VM_jump,

		// 包含额外的4字节，用于表示get成员名字在常亮表中的索引
		VM_getMember,

		VM_doCall,
		VM_getIndex,

		// 创建基本类型元素的指令
		// 创建的数据被压入栈
		// 其中指令的第二个字节表示创建的是常量还是变量
		// 除了VM_createDouble后面跟8个字节，其余后面都跟4个字节表示创建的数据值
		VM_createchar,
		VM_createByte,
		VM_createShort,
		VM_createUshort,
		VM_createInt,
		VM_createUint,
		VM_createFloat,
		VM_createDouble,
		VM_createString,
		VM_createArray,

		// 将距离当前栈顶指定偏移的数据再次压入栈顶，偏移由Instrction的data提供（
		// 故不能超过65535）
		VM_push,
		VM_pop,

		// 带有栈帧标志的push
		// 此时Instruction的data域指定了向上递推的栈帧个数（基于0）
		// 额外的一个uint32_t用于指示基于该栈帧的元素偏移
		// 类似于间接地址访问
		VM_copyAtFrame,

		// 栈帧的维护，最多有65536*256个栈帧，目前本虚拟机还不支持子过程调用
		// 仅支持对象的功能调用。但是，将来支持子过程调用的话，也将使用栈帧来
		// 维护调用栈，所以，最终只支持这么多级的调用
		VM_pushStackFrame,
		VM_popStackFrame,

		VM_end,

		// data域指示参数个数（这样，不超过65535）
		VM_setParamCount,

		// 调试指令1，占用一个指令长（4字节）+额外的4字节备用
		VM_debug1,

		// 为了方便编译代码，还是加上了本指令，用于和VM_jz相对
		VM_jnz,

		// 为了方便测试，增加本指令，用于退出应用程序的执行
		VM_debugbreak,

		// 创建函数对象，此时，4字节的额外数据指向了函数对象的说明区域
		// 说明区域是一个函数对象的说明结构，struct FunctionDesc
		// 调用函数时，会处理栈帧，而从函数返回时也会从栈帧中弹出最顶的元素
		// 恢复之前的执行栈
		VM_createFunction,

		// 从函数调用返回
		VM_return,
	};

#pragma pack(push,1)
	struct Instruction
	{
		uint32_t code : 8;
		uint32_t extCode : 8;
		uint32_t data : 16;
	};

	struct FunctionDesc
	{
		// 函数的字节长度，必须是4的整倍数
		uint32_t len;
		// 函数的名称id，如果为0，表示这个函数是匿名函数
		uint32_t stringId;

		// 函数的参数个数
		uint32_t paramCount;
	};
#pragma pack(pop)
	typedef uint32_t CommonInstruction;
}

namespace scriptAPI { class ScriptCompiler; }

namespace compiler
{
	class ConstStringData
	{
		// 常量字符串区域，每一项都是一个索引，指示在索引表中的序号
		// 索引表在加载程序时读入。索引表
		struct StringIndex
		{
			// 字符串在常量区域中的偏移
			uint32_t offset;
			// 字符串的长度（不包含0结尾字符）
			uint32_t size;
		};
		// 索引表
		std::vector<StringIndex> mConstStringIndexTable;
		// 当编译器读取到一个字符串，用这个索引表来快速确定是否已经存在该字符串
		std::map<std::string,size_t> mIndexTable;

		// 存放所有字符数据的内存块
		std::string mStringBuffer;

	public:
		ConstStringData();
		void Clear();
		size_t RegistString(const std::string &str);
		bool GetString(size_t i, std::string &s) const;

		int SaveConstStringDataToFile(FILE *file) const;
		int LoadConstStringDataFromFile(FILE *file);
	};
	
	class GenerateInstructionHelper;
	class CompileResult
	{
		friend class GenerateInstructionHelper;
		friend class runtime::runtimeContext;
		friend class scriptAPI::ScriptCompiler;
		friend class runtime::FunctionObject;

	public:
		typedef std::vector<uint32_t> ScriptCode;

	private:
		// 代码区域，代码大小一定是4字节的整倍数（编译时不够4字节会填充)	
		ScriptCode mCode;

		ConstStringData *mConstStringData;

	private:
		inline ScriptCode& GetCode() { return mCode; }
		inline ConstStringData* GetStringData() { return mConstStringData; }
		// 得到当前的代码末尾位置
		void Clear();

	public:
		CompileResult(ConstStringData *stringData);
		~CompileResult();

		uint32_t SaveCurrentCodePosition() const { return mCode.size(); }

		inline int SaveConstStringDataToFile(FILE *file) const
		{
			return mConstStringData->SaveConstStringDataToFile(file);
		}
		inline int LoadConstStringDataFromFile(FILE *file)
		{
			return mConstStringData->LoadConstStringDataFromFile(file);
		}
		
		int SaveCodeToFile(FILE *file) const;
		int LoadCodeFromFile(FILE *file);
	};

	class GenerateInstructionHelper
	{
	private:
		CompileResult *mCompileResult;
		compiler::CompileResult::ScriptCode &mCode;

	public:
		GenerateInstructionHelper(CompileResult *cr)
			: mCompileResult(cr)
			, mCode(cr->mCode)
		{
		}

		inline void SetCode(uint32_t pos, uint32_t val)
		{
			mCode[pos] = val;
		}

		inline uint32_t RegistName(const char *name)
		{
			return mCompileResult->GetStringData()->RegistString(name);
		}

		// 直接在代码中插入字符串，返回first是插入的位置，second是基于uint32_t的长度
		// 这些可见的字符便于查看代码时，较易定位
		std::pair<uint32_t,uint32> InsertStringDataToCode(const char *str)
		{
			uint32_t l = (uint32_t)strlen(str);
			uint32_t actInsertLen = (l + 3) / 4;
			uint32_t sNow = mCode.size();
			mCode.resize(sNow + actInsertLen);
			memcpy(&mCode[sNow], str, l + 1);
			return std::make_pair(sNow, actInsertLen);
		}

		void Insert_add_Instruction()
		{
			mCode.push_back(runtime::VM_add);
		}

		void Insert_sub_Instruction()
		{
			mCode.push_back(runtime::VM_sub);
		}

		void Insert_mul_Instruction()
		{
			mCode.push_back(runtime::VM_mul);
		}

		void Insert_div_Instruction()
		{
			mCode.push_back(runtime::VM_div);
		}

		void Insert_mod_Instruction()
		{
			mCode.push_back(runtime::VM_mod);
		}

		void Insert_logicAnd_Instruction()
		{
			mCode.push_back(runtime::VM_logicAnd);
		}

		void Insert_logicOr_Instruction()
		{
			mCode.push_back(runtime::VM_logicOr);
		}

		void Insert_logicNot_Instruction()
		{
			mCode.push_back(runtime::VM_logicNot);
		}

		void Insert_bitwiseAnd_Instruction()
		{
			mCode.push_back(runtime::VM_bitwiseAnd);
		}

		void Insert_bitwiseOr_Instruction()
		{
			mCode.push_back(runtime::VM_bitwiseOr);
		}

		void Insert_bitwiseXOR_Instruction()
		{
			mCode.push_back(runtime::VM_bitwiseXOR);
		}

		void Insert_bitwiseNot_Instruction()
		{
			mCode.push_back(runtime::VM_bitwiseNot);
		}

		void Insert_setVal_Instruction()
		{
			mCode.push_back(runtime::VM_setVal);
		}

		void Insert_equal_Instruction()
		{
			mCode.push_back(runtime::VM_equal);
		}

		void Insert_notEqual_Instruction()
		{
			mCode.push_back(runtime::VM_notEqual);
		}

		void Insert_greater_Instruction()
		{
			mCode.push_back(runtime::VM_greater);
		}

		void Insert_less_Instruction()
		{
			mCode.push_back(runtime::VM_less);
		}

		void Insert_greaterEqual_Instruction()
		{
			mCode.push_back(runtime::VM_greaterEqual);
		}

		void Insert_lessEqual_Instruction()
		{
			mCode.push_back(runtime::VM_lessEqual);
		}

		// 这两个跳转指令插入函数的参数是跳转目标，如果不确定就填写任何数
		// 他们的返回值是跳转目标地址的指令偏移位置
		uint32_t Insert_jz_Instruction(uint32_t target)
		{
			mCode.push_back(runtime::VM_jz);
			uint32_t r = mCompileResult->SaveCurrentCodePosition();
			mCode.push_back(target);
			return r;
		}

		uint32_t Insert_jump_Instruction(uint32_t target)
		{
			mCode.push_back(runtime::VM_jump);
			uint32_t r = mCompileResult->SaveCurrentCodePosition();
			mCode.push_back(target);
			return r;
		}

		void Insert_getMember_Instruction(const char *name)
		{
			mCode.push_back(runtime::VM_getMember);
			mCode.push_back(
				static_cast<uint32_t>(
				mCompileResult->GetStringData()->RegistString(name)
				));
		}

		void Insert_doCall_Instruction()
		{
			mCode.push_back(runtime::VM_doCall);
		}

		void Insert_getIndex_Instruction()
		{
			mCode.push_back(runtime::VM_getIndex);
		}

		void Insert_createChar_Instruction(uint32_t v)
		{
			mCode.push_back(runtime::VM_createchar);
			mCode.push_back(v);
		}

		void Insert_createByte_Instruction(uint32_t v)
		{
			mCode.push_back(runtime::VM_createByte);
			mCode.push_back(v);
		}

		void Insert_createShort_Instruction(uint32_t v)
		{
			mCode.push_back(runtime::VM_createShort);
			mCode.push_back(v);
		}

		void Insert_createUshort_Instruction(uint32_t v)
		{
			mCode.push_back(runtime::VM_createUshort);
			mCode.push_back(v);
		}

		void Insert_createInt_Instruction(int v)
		{
			mCode.push_back(runtime::VM_createInt);
			mCode.push_back(*reinterpret_cast<uint32_t*>(&v));
		}

		void Insert_createUint_Instruction(uint32_t v)
		{
			mCode.push_back(runtime::VM_createUint);
			mCode.push_back(v);
		}

		void Insert_createFloat_Instruction(float v)
		{
			mCode.push_back(runtime::VM_createFloat);
			mCode.push_back(*reinterpret_cast<uint32_t*>(&v));
		}

		void Insert_createDouble_Instruction(double v)
		{
			// 这条指令将来会废弃，现在也没有用到，先占个位置吧
			assert(0);
			mCode.push_back(runtime::VM_createDouble);
			mCode.resize(mCode.size() + 2);
			*reinterpret_cast<double*>(&mCode[mCode.size() - 2]) = v;
		}

		void Insert_createString_Instruction(const char *name)
		{
			mCode.push_back(runtime::VM_createString);
			mCode.push_back(
				static_cast<uint32_t>(
				mCompileResult->GetStringData()->RegistString(name)
				));
		}

		void Insert_createArray_Instruction()
		{
			mCode.push_back(runtime::VM_createArray);
			mCode.push_back(0);
		}

		void Insert_push_Instruction(uint16_t off)
		{
			runtime::Instruction inst;
			inst.code = runtime::VM_push;
			inst.data = off;
			mCode.push_back(*reinterpret_cast<uint32_t*>(&inst));
		}

		void Insert_pop_Instruction()
		{
			mCode.push_back(runtime::VM_pop);
		}

		void Insert_copyAtFrame_Instruction(uint16_t frameOff, uint32_t index)
		{
			runtime::Instruction inst;
			inst.code = runtime::VM_copyAtFrame;
			inst.data = frameOff;
			mCode.push_back(*reinterpret_cast<uint32_t*>(&inst));
			mCode.push_back(index);
		}

		void Insert_pushStackFrame_Instruction()
		{
			mCode.push_back(runtime::VM_pushStackFrame);
		}
		
		void Insert_popStackFrame_Instruction()
		{
			mCode.push_back(runtime::VM_popStackFrame);
		}

		void Insert_end_Instruction()
		{
			mCode.push_back(runtime::VM_end);
		}

		void Insert_setParamCount_Instruction(uint16_t paramCount)
		{
			runtime::Instruction inst;
			inst.code = runtime::VM_setParamCount;
			inst.data = paramCount;
			mCode.push_back(*reinterpret_cast<uint32_t*>(&inst));
		}

		void Insert_debug1_Instruction(uint32_t param = 0)
		{
			mCode.push_back(runtime::VM_debug1);
			mCode.push_back(param);
		}

		uint32_t Insert_jnz_Instruction(uint32_t target)
		{
			mCode.push_back(runtime::VM_jnz);
			uint32_t r = mCompileResult->SaveCurrentCodePosition();
			mCode.push_back(target);
			return r;
		}

		void Insert_debugbreak_Instruction(uint32_t param = 0)
		{
			mCode.push_back(runtime::VM_debugbreak);
			mCode.push_back(param);
		}

		uint32_t Insert_createFunction_Instruction()
		{
			mCode.push_back(runtime::VM_createFunction);
			uint32_t r = mCompileResult->SaveCurrentCodePosition();
			mCode.push_back(0);
			return r;
		}

		void InsertFunctionDesc(runtime::FunctionDesc *funcDesc)
		{
			uint32_t s = mCode.size();
			mCode.resize(s + sizeof(runtime::FunctionDesc) / 4);
			*reinterpret_cast<runtime::FunctionDesc*>(&mCode[s]) = *funcDesc;
		}

		void Insert_return_Instruction()
		{
			mCode.push_back(runtime::VM_return);
		}
	};
}
