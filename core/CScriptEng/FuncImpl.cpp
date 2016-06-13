#include "stdafx.h"
#include "vm.h"

using namespace runtime;

FunctionObject::FunctionObject()
	: mInstHead(NULL)
	, mInstTail(NULL)
	, mFuncName(new std::string)
{
	memset(&mFuncDesc, 0, sizeof(mFuncDesc));
}

FunctionObject::~FunctionObject()
{
	delete mFuncName;
}

bool FunctionObject::ReadFuncHeader(uint32_t *codeHeader, uint32_t *codeTail, 
	uint32_t off, runtimeContext *context)
{
	// 代码超出了界限
	if (codeHeader + off + sizeof(FunctionDesc) / sizeof(uint32_t) > codeTail)
		return false;

	FunctionDesc *fd = reinterpret_cast<FunctionDesc*>(
		codeHeader + off);
	mFuncDesc = *fd;				

	if (mFuncDesc.len / 4 * 4 != mFuncDesc.len)
		return false;

	mContext = context;
	mInstHead = codeHeader + off;
	mInstTail = mInstHead + mFuncDesc.len / sizeof(uint32_t);
	if (mInstTail > codeTail)
		return false;

	if (mFuncDesc.stringId)
	{
		if (!mContext->GetCompileResult()->GetStringData()->GetString(
			mFuncDesc.stringId, *mFuncName))
			return false;
	}

	mInstHead += sizeof(FunctionDesc) / sizeof(uint32_t);

	return true;
}

uint32_t FunctionObject::GetObjectTypeId() const
{
	return DT_function;
}

runtimeObjectBase* FunctionObject::Add(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* FunctionObject::Sub(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* FunctionObject::Mul(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* FunctionObject::Div(const runtimeObjectBase *obj)
{
	return NULL;
}

runtimeObjectBase* FunctionObject::SetValue(runtimeObjectBase *obj)
{
	if (obj->GetObjectTypeId() != DT_function)
		return NULL;

	const FunctionObject *fo = static_cast<const FunctionObject*>(obj);
	mFuncDesc = fo->mFuncDesc;
	mInstHead = fo->mInstHead;
	mInstTail = fo->mInstTail;
	mContext = fo->mContext;
	*mFuncName = *fo->mFuncName;

	return this;
}

runtimeObjectBase* FunctionObject::GetMember(const char *memName)
{
	return baseTypeObject::GetMember(memName);
}

runtimeObjectBase* FunctionObject::doCall(doCallContext *context)
{
	int calRet;
	// 参数栈中的参数个数超过了函数声明能够处理的参数个数了
	if (context->GetParamCount() > mFuncDesc.paramCount)
	{
		SCRIPT_TRACE("FunctionObject::doCall: "
			"param count of function [%s] does not match.\n", mFuncName->c_str());
		return NULL;
	}

	// 保存执行环境
	uint32_t *pcSaved = mContext->mPC;
	uint32_t *sectionHeaderSaved = mContext->mSectionHeader;
	uint32_t *pcEndSaved = mContext->mPCEnd;
	uint32_t paramCountSaved = mContext->mParamCount;
	uint32_t callLayerSaved = mContext->mCallStackLayer;

	mContext->OnInst_pushStackFrame(NULL, NULL, 0);
	scriptAPI::ScriptCompiler::CompileCode cc;
	cc.code = mInstHead;
	cc.sizeInUint32 = static_cast<decltype(cc.sizeInUint32)>(mInstTail - mInstHead);
	uint32_t tempStackPos = mContext->mCurrentStack;
	for (uint32_t pi = 0; pi < mFuncDesc.paramCount; pi++)
	{
		mContext->PushObject(mContext->mRuntimeStack[tempStackPos - mFuncDesc.paramCount + pi]);
	}
	mContext->mCallStackLayer++;
	if ((calRet = mContext->Execute(&cc, mContext->GetCompileResult())) != -100)
	{
		return NULL;
	}
	// 当前栈顶元素就是返回值，保存它
	runtimeObjectBase *o = mContext->mA;
	mContext->OnInst_popStackFrame(NULL, NULL, 0);

	// 恢复当前级别的执行环境
	mContext->mPC = pcSaved;
	mContext->mSectionHeader = sectionHeaderSaved;
	mContext->mPCEnd = pcEndSaved;
	mContext->mParamCount = paramCountSaved;
	mContext->mCallStackLayer = callLayerSaved;
	o->ReleaseNotDelete();
	return o;
}

runtimeObjectBase* FunctionObject::getIndex(int i)
{
	return NULL;
}

stringObject* FunctionObject::toString()
{
	stringObject *s = new runtime::ObjectModule<stringObject>;
	s->mVal->append(*mFuncName);
	return s;
}

bool FunctionObject::isGreaterThan(const runtimeObjectBase *obj)
{
	return false;
}

bool FunctionObject::isEqual(const runtimeObjectBase *obj)
{
	return false;
}
