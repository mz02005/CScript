#pragma once
#include "CScriptEng.h"
#include "scriptDef.h"

namespace runtime {
	class FunctionObject;

	struct VMConfig
	{
		uint32_t stackSize;
		uint32_t stackFrameSize;

		VMConfig();

		void Normalize();
	};

	class runtimeContext : public doCallContext
	{
		friend class FunctionObject;

	private:
		// 调用堆栈
		std::vector<runtimeObjectBase*> mRuntimeStack;
		uint32_t mCurrentStack;

		// 记录栈帧
		std::vector<uint32_t> mStackFrame;
		uint32_t mStackFrameSize;
		std::vector<std::pair<uint32_t*,uint32_t*> > mPCFrame;

		// 记录块帧，用于释放块内对象的内存
		std::vector<uint32_t> mBlockFrame;
		uint32_t mBlockFrameSize;

		// 记录当前doCall的参数个数
		uint32_t mParamCount;

		// 临时寄存器，保存return之前的返回值
		runtimeObjectBase *mA;

		compiler::CompileResult *mCompileResult;

		// 当前栈帧的代码头
		uint32_t *mSectionHeader;
		// 指令指针
		uint32_t *mPC;
		// 指令尾部
		uint32_t *mPCEnd;

		uint32_t mCallStackLayer;

		VMConfig mConfig;
		
	private:
		void SetCompareResult(bool r);

		void GetOrigPC(uint32_t *&header, uint32_t *&tail) const;
		
		static const int mBlockFrameSectionSize = 4096;
		void CheckForBlockFrameSize();

		static inline bool isZero(runtimeObjectBase *base)
		{
			switch (base->GetObjectTypeId())
			{
			case DT_int8:
				return getObjectData<charObject>(base) == 0;
			case DT_int16:
				return getObjectData<charObject>(base) == 0;
			case DT_int32:
				return getObjectData<charObject>(base) == 0;
			case DT_uint8:
				return getObjectData<charObject>(base) == 0;
			case DT_uint16:
				return getObjectData<charObject>(base) == 0;
			case DT_uint32:
				return getObjectData<charObject>(base) == 0;
			default:
				break;
			}

			return false;
		}

		template <typename T, typename Oper>
		inline void bitwiseOper(runtimeObjectBase *o, runtimeObjectBase *p)
		{
			runtimeObjectBase *r = new runtime::ObjectModule<T>;
			static_cast<T*>(r)->mVal = Oper()(runtime::getObjectData<T>(o), runtime::getObjectData<T>(p));
			o->Release();
			p->Release();
			mCurrentStack -= 2;
			PushObject(r);
		}

		template <typename T, typename Oper>
		inline void OneOperatorBitwiseOperation(runtimeObjectBase *o)
		{
			runtimeObjectBase *r = new runtime::ObjectModule<T>;
			static_cast<T*>(r)->mVal = Oper()(getObjectData<T>(o));
			mRuntimeStack[--mCurrentStack - 1]->Release();
			PushObject(r);
		}

		template <template <class T> class Oper>
		inline int bitwiseOperInner(runtimeObjectBase *o, runtimeObjectBase *p)
		{
			switch (o->GetObjectTypeId())
			{
			case DT_int8:
				bitwiseOper<charObject,Oper<int8_t> >(o, p);
				break;

			case DT_int16:
				bitwiseOper<shortObject,Oper<int16_t> >(o, p);
				break;

			case DT_int32:
				bitwiseOper<intObject,Oper<int32_t> >(o, p);
				break;

			case DT_uint8:
				bitwiseOper<byteObject,Oper<uint8_t> >(o, p);
				break;

			case DT_uint16:
				bitwiseOper<ushortObject,Oper<uint16_t> >(o, p);
				break;

			case DT_uint32:
				bitwiseOper<uintObject,Oper<uint32_t> >(o, p);
				break;

			default:
				SCRIPT_TRACE("runtimeContext::OnInst_bitwiseAnd: invalid bitwise operator.\n");
				break;
			}
			return -1;
		}

	private:
		int OnInvalidInstruction(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_createInt(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_createFloat(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_createString(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_createArray(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_pop(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_add(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_sub(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_mul(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_div(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_mod(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_bitwiseAnd(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_bitwiseOr(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_bitwiseXOR(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_bitwiseNot(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_logicAnd(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_logicOr(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_logicNot(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_end(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_pushStackFrame(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_popStackFrame(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_copyAtFrame(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_setVal(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_push(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_getMember(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_doCall(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_setParamCount(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_getIndex(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_equal(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_notEqual(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_greater(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_less(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_greaterEqual(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_lessEqual(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_jnz(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_jz(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_jump(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_debug1(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_debugbreak(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_createFunction(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_return(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_copyAtFrame2(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_popStackFrameAndSaveResult(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

		int OnInst_loadData(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_enterBlock(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_leaveBlock(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		int OnInst_saveToA(Instruction *inst, uint8_t *moreData, uint32_t moreSize);

	private:
		int RunInner();

	private:
		typedef int(runtimeContext::*OnInstruction)(Instruction *inst, uint8_t *moreData, uint32_t moreSize);
		static const struct InstructionEntry
		{
			OnInstruction inst;
			uint32_t moreSize;
		};
		static const InstructionEntry mIES[256];

	public:
		compiler::CompileResult* GetCompileResult() { return mCompileResult; }
		int PushObject(runtimeObjectBase *obj);
		int ReplaceObject(int pos, runtimeObjectBase *obj);

		runtimeContext(VMConfig *config);
		~runtimeContext();

		int Execute(compiler::CompileResult *compileResult);
		int Execute(void *code, compiler::CompileResult *compileResult, bool recoveryStack = false);

		// doCallContext
		virtual uint32_t GetParamCount();
		virtual runtimeObjectBase* GetParam(uint32_t i);
		virtual double GetDoubleParam(uint32_t i);
		virtual float GetFloatParam(uint32_t i);
		virtual uint32_t GetUint32Param(uint32_t i);
		virtual int32_t GetInt32Param(uint32_t i);
		virtual uint16_t GetUint16Param(uint32_t i);
		virtual int16_t GetInt16Param(uint32_t i);
		virtual uint8_t GetUint8Param(uint32_t i);
		virtual int8_t GetInt8Param(uint32_t i);
		virtual const char* GetStringParam(uint32_t i);
		virtual runtimeObjectBase* GetObject(uint32_t i);

		virtual uint32_t GetArrayParamElemCount(uint32_t i);
		virtual double GetDoubleElemOfArrayParam(uint32_t i, uint32_t e);
		virtual float GetFloatElemOfArrayParam(uint32_t i, uint32_t e);
		virtual uint32_t GetUint32ElemOfArrayParam(uint32_t i, uint32_t e);
		virtual int32_t GetInt32ElemOfArrayParam(uint32_t i, uint32_t e);
		virtual uint16_t GetUint16ElemOfArrayParam(uint32_t i, uint32_t e);
		virtual int16_t GetInt16ElemOfArrayParam(uint32_t i, uint32_t e);
		virtual uint8_t GetUint8ElemOfArrayParam(uint32_t i, uint32_t e);
		virtual int8_t GetInt8ElemOfArrayParam(uint32_t i, uint32_t e);
		virtual const char* GetStringElemOfArrayParam(uint32_t i, uint32_t e);
		virtual runtimeObjectBase* GetObjectOfArrayParam(uint32_t i, uint32_t e);
	};
}
