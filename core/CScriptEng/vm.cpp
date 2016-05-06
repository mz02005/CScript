#include "stdafx.h"
#include "vm.h"
#include "compile.h"
#include "CScriptEng.h"
#include "arrayType.h"
#include "rtlib.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

VMConfig::VMConfig()
	: stackSize(1024)
	, stackFrameSize(512)
{
}

void VMConfig::Normalize()
{
	if (stackSize < 128)
		stackSize = 128;
	if (stackFrameSize < 128)
		stackFrameSize = 128;
}

///////////////////////////////////////////////////////////////////////////////

uint32_t baseObjDefault::GetObjectTypeId() const
{
	return DT_UserTypeBegin;
}

runtimeObjectBase* baseObjDefault::Add(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* baseObjDefault::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* baseObjDefault::Mul(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* baseObjDefault::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* baseObjDefault::SetValue(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* baseObjDefault::GetMember(const char *memName)
{
	return nullptr;
}

runtimeObjectBase* baseObjDefault::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* baseObjDefault::getIndex(int i)
{
	return nullptr;
}

stringObject* baseObjDefault::toString()
{
	return nullptr;
}

bool baseObjDefault::isGreaterThan(const runtimeObjectBase *obj)
{
	return false;
}

bool baseObjDefault::isEqual(const runtimeObjectBase *obj)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////

namespace runtime
{
	class toStringObject : public runtime::baseObjDefault
	{
		friend class baseTypeObject;
		baseTypeObject *mBaseObj;

	public:
		toStringObject()
			: mBaseObj(nullptr)
		{
		}

		virtual uint32_t GetObjectTypeId() const
		{
			return runtime::DT_UserTypeBegin;
		}

		virtual runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			return mBaseObj ? mBaseObj->toString() : nullptr;
		}
	};
}

baseTypeObject::baseTypeObject()
	: mIsConst(false)
{
}

baseTypeObject::~baseTypeObject()
{
}

runtimeObjectBase* baseTypeObject::GetMember(const char *memName)
{
	if (!strcmp("toString", memName))
	{
		toStringObject *s = new runtime::ContainModule<toStringObject>(this);
		s->mBaseObj = this;
		return s;
	}
	return __super::GetMember(memName);
}

///////////////////////////////////////////////////////////////////////////////

runtimeContext::runtimeContext(VMConfig *config)
	: mPC(nullptr)
	, mPCEnd(nullptr)
	, mCurrentStack(0)
	, mParamCount(0)
{
	if (config) {
		mConfig = *config;
		mConfig.Normalize();
	}

	mRuntimeStack.resize(mConfig.stackSize, nullptr);

	mStackFrame.resize(mConfig.stackFrameSize);
	mStackFrame[0] = 0;
	mStackFrameSize = 1;

	PushObject(new runtime::ObjectModule<runtime::CreateArrayObj>);
	rtLibHelper::RegistRuntimeObjs(this);
}

runtimeContext::~runtimeContext()
{
	for (uint32_t x = 0; x < mCurrentStack; x++)
		mRuntimeStack[x]->Release();
}

uint32_t runtimeContext::GetParamCount()
{
	return mParamCount;
}

runtimeObjectBase* runtimeContext::GetParam(uint32_t i)
{
	if (i >= mParamCount)
		return nullptr;

	return mRuntimeStack[mCurrentStack - mParamCount + i];
}

double runtimeContext::GetDoubleParam(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetDoubleParam: out of range\n");

	double r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetDoubleParam: invalid data type.\n");
	r = getObjectDataOrig<double>(o);
	return r;
}

float runtimeContext::GetFloatParam(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetFloatParam: out of range\n");

	float r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetFloatParam: invalid data type.\n");
	r = getObjectDataOrig<float>(o);
	return r;
}

uint32_t runtimeContext::GetUint32Param(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetUint32Param: out of range\n");

	uint32_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetUint32Param: invalid data type.\n");
	r = getObjectDataOrig<uint32_t>(o);
	return r;
}

int32_t runtimeContext::GetInt32Param(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetInt32Param: out of range\n");
	
	int32_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetInt32Param: invalid data type.\n");
	r = getObjectDataOrig<int32_t>(o);
	return r;
}

uint16_t runtimeContext::GetUint16Param(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetUint16Param: out of range\n");

	uint16_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetUint16Param: invalid data type.\n");
	r = getObjectDataOrig<uint16_t>(o);
	return r;
}

int16_t runtimeContext::GetInt16Param(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetInt16Param: out of range\n");

	int16_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetInt16Param: invalid data type.\n");
	r = getObjectDataOrig<int16_t>(o);
	return r;
}

uint8_t runtimeContext::GetUint8Param(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetUint8Param: out of range\n");

	uint8_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetUint8Param: invalid data type.\n");
	r = getObjectDataOrig<uint8_t>(o);
	return r;
}

int8_t runtimeContext::GetInt8Param(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetInt8Param: out of range\n");

	int8_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (!isNumberType(o))
		throw std::exception("runtimeContext::GetInt8Param: invalid data type.\n");
	r = getObjectDataOrig<int8_t>(o);
	return r;
}

const char* runtimeContext::GetStringParam(uint32_t i) {
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetInt8Param: out of range\n");

	std::string *r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_string)
	{
		return nullptr;
	}

	r = static_cast<stringObject*>(o)->mVal;

	return r->c_str();
}

uint32_t runtimeContext::GetArrayParamElemCount(uint32_t i)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetArrayParamElemCount: out of range\n");

	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];
	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetArrayParamElemCount: not array");

	return static_cast<arrayObject*>(o)->mData->size();
}

double runtimeContext::GetDoubleElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetDoubleElemOfArrayParam: out of range\n");

	double r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetDoubleElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetDoubleElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetDoubleElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<double>(sub);
	return r;
}

float runtimeContext::GetFloatElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetFloatElemOfArrayParam: out of range\n");

	float r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetFloatElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetFloatElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetFloatElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<float>(sub);
	return r;
}

uint32_t runtimeContext::GetUint32ElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetUint32ElemOfArrayParam: out of range\n");

	uint32_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetUint32ElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetUint32ElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetUint32ElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<uint32_t>(sub);
	return r;
}

int32_t runtimeContext::GetInt32ElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetInt32ElemOfArrayParam: out of range\n");

	int32_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetInt32ElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetInt32ElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetInt32ElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<int32_t>(sub);
	return r;
}

uint16_t runtimeContext::GetUint16ElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetUint16ElemOfArrayParam: out of range\n");

	uint16_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetUint16ElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetUint16ElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetUint16ElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<uint16_t>(sub);
	return r;
}

int16_t runtimeContext::GetInt16ElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetInt16ElemOfArrayParam: out of range\n");

	int16_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetInt16ElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetInt16ElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetInt16ElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<int16_t>(sub);
	return r;
}

uint8_t runtimeContext::GetUint8ElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetUint8ElemOfArrayParam: out of range\n");

	uint8_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetUint8ElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetUint8ElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetUint8ElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<uint8_t>(sub);
	return r;
}

int8_t runtimeContext::GetInt8ElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetInt8ElemOfArrayParam: out of range\n");

	int8_t r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetInt8ElemOfArrayParam: not array");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetInt8ElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);

	if (!isNumberType(sub))
		throw std::exception("runtimeContext::GetInt8ElemOfArrayParam: invalid data type.\n");
	r = getObjectDataOrig<int8_t>(sub);
	return r;
}

const char* runtimeContext::GetStringElemOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetStringElemOfArrayParam: out of range\n");

	std::string *r;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetStringElemOfArrayParam: not array\n");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetStringElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);
	if (sub->GetObjectTypeId() != DT_string)
		throw std::exception("runtimeContext::GetStringElemOfArrayParam: invalid data type.\n");

	r = static_cast<stringObject*>(sub)->mVal;

	return r->c_str();
}

int runtimeContext::OnInvalidInstruction(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	SCRIPT_TRACE("Invalid instruction %d\n", (int)inst->code);
	return -1;
}

int runtimeContext::OnInst_push(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (inst->data >= mCurrentStack)
	{
		SCRIPT_TRACE("OnInst_push: stack out of range.\n");
		return -1;
	}
	mRuntimeStack[mCurrentStack] = mRuntimeStack[mCurrentStack - 1 - inst->data];
	mRuntimeStack[mCurrentStack++]->AddRef();
	return 0;
}

int runtimeContext::OnInst_getIndex(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("OnInst_getIndex: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *a = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *i = mRuntimeStack[mCurrentStack - 1];
	int index;
	switch (i->GetObjectTypeId())
	{
	default:
		SCRIPT_TRACE("OnInst_getIndex: invalid index data type.\n");
		return -1;

	case DT_int32:
	case DT_int16:
	case DT_int8:
	case DT_uint32:
	case DT_uint16:
	case DT_uint8:
		index = getObjectData<intObject>(i);
		break;
	}
	i->Release();
	runtimeObjectBase *r = a->getIndex(index);
	if (!r)
	{
		SCRIPT_TRACE("OnInst_getIndex: object does not support getIndex operation.\n");
		return -1;
	}
	mCurrentStack -= 2;
	PushObject(r);
	a->Release();

	return 0;
}

int runtimeContext::OnInst_doCall(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 1 + mParamCount)
	{
		SCRIPT_TRACE("OnInst_doCall: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *r = mRuntimeStack[mCurrentStack - 1 - mParamCount]->doCall(static_cast<doCallContext*>(this));
	if (!r)
	{
		SCRIPT_TRACE("OnInst_doCall: fail.\n");
		return -1;
	}

	r->AddRef();

	// 释放参数
	for (uint32_t i = 0; i < mParamCount; i++)
		mRuntimeStack[mCurrentStack - 1 - i]->Release();
	mCurrentStack -= mParamCount;
	mRuntimeStack[--mCurrentStack]->Release();
	mRuntimeStack[mCurrentStack++] = r;
	return 0;
}

int runtimeContext::OnInst_getMember(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 1) {
		SCRIPT_TRACE("OnInst_getMember: stack out of range.\n");
		return -1;
	}

	std::string s;
	bool r = mCompileResult->GetStringData()->GetString(*reinterpret_cast<uint32_t*>(moreData), s);
	if (!r) {
		SCRIPT_TRACE("OnInst_getMemger: get string [%s] fail.\n",
			s.c_str());
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 1]->GetMember(s.c_str());
	if (!o) {
		SCRIPT_TRACE("OnInst_getMember: fail.\n");
		return -1;
	}
	o->AddRef();
	mRuntimeStack[--mCurrentStack]->Release();
	mRuntimeStack[mCurrentStack++] = o;
	return 0;
}

int runtimeContext::OnInst_setParamCount(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	mParamCount = inst->data;
	return 0;
}

int runtimeContext::OnInst_pop(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (!mCurrentStack)
	{
		SCRIPT_TRACE("OnInst_pop: Stack out of range.\n");
		return -1;
	}
	mRuntimeStack[--mCurrentStack]->Release();
	return 0;
}

int runtimeContext::OnInst_add(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2)
	{
		SCRIPT_TRACE("OnInst_add: Stack outof range\n");
		return -1;
	}

	runtimeObjectBase *r = mRuntimeStack[mCurrentStack - 2]->Add(mRuntimeStack[mCurrentStack - 1]);
	if (!r) {
		SCRIPT_TRACE("Invalid add operation between type [%u] and [%u].\n", 
		mRuntimeStack[mCurrentStack - 2]->GetObjectTypeId(),
		mRuntimeStack[mCurrentStack - 1]->GetObjectTypeId());
		return -1;
	}

	mRuntimeStack[mCurrentStack - 2]->Release();
	mRuntimeStack[mCurrentStack - 1]->Release();
	mRuntimeStack[mCurrentStack - 2] = r;
	r->AddRef();
	mCurrentStack--;
	return 0;
}

int runtimeContext::OnInst_sub(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2)
	{
		SCRIPT_TRACE("OnInst_sub: Stack outof range\n");
		return -1;
	}

	runtimeObjectBase *r = mRuntimeStack[mCurrentStack - 2]->Sub(mRuntimeStack[mCurrentStack - 1]);
	if (!r) {
		SCRIPT_TRACE("Invalid sub operation between type [%u] and [%u].\n", 
		mRuntimeStack[mCurrentStack - 2]->GetObjectTypeId(),
		mRuntimeStack[mCurrentStack - 1]->GetObjectTypeId());
		return -1;
	}

	mRuntimeStack[mCurrentStack - 2]->Release();
	mRuntimeStack[mCurrentStack - 1]->Release();
	mRuntimeStack[mCurrentStack - 2] = r;
	r->AddRef();
	mCurrentStack--;
	return 0;
}

int runtimeContext::OnInst_mul(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2)
	{
		SCRIPT_TRACE("OnInst_mul: Stack outof range\n");
		return -1;
	}

	runtimeObjectBase *r = mRuntimeStack[mCurrentStack - 2]->Mul(mRuntimeStack[mCurrentStack - 1]);
	if (!r) {
		SCRIPT_TRACE("Invalid mul operation between type [%u] and [%u].\n", 
		mRuntimeStack[mCurrentStack - 2]->GetObjectTypeId(),
		mRuntimeStack[mCurrentStack - 1]->GetObjectTypeId());
		return -1;
	}

	mRuntimeStack[mCurrentStack - 2]->Release();
	mRuntimeStack[mCurrentStack - 1]->Release();
	mRuntimeStack[mCurrentStack - 2] = r;
	r->AddRef();
	mCurrentStack--;
	return 0;
}

template <typename T>
inline runtimeObjectBase* doMod(runtimeObjectBase *o, runtimeObjectBase *p)
{
	typedef runtime::ObjectModule<T> TypeToCreate;
	runtimeObjectBase *r = new TypeToCreate;
	static_cast<T*>(r)->mVal = getObjectDataOrig<T::InnerDataType>(o) % getObjectDataOrig<T::InnerDataType>(p);
	return r;
}

int runtimeContext::OnInst_mod(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("runtimeContext::OnInst_mod: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *p = mRuntimeStack[mCurrentStack - 1];
	runtimeObjectBase *r;

	switch (o->GetObjectTypeId())
	{
	case DT_int8:
		r = doMod<charObject>(o, p);
		break;
	case DT_uint8:
		r = doMod<byteObject>(o, p);
		break;
	case DT_int16:
		r = doMod<shortObject>(o, p);
		break;
	case DT_uint16:
		r = doMod<ushortObject>(o, p);
		break;
	case DT_int32:
		r = doMod<intObject>(o, p);
		break;
	case DT_uint32:
		r = doMod<uintObject>(o, p);
		break;

	default:
		SCRIPT_TRACE("runtimeContext::OnInst_mod: mod on invalid type [%d] and [%d].\n",
			(int)o->GetObjectTypeId(), (int)p->GetObjectTypeId());
		return -1;
	}
	mRuntimeStack[mCurrentStack - 2]->Release();
	mRuntimeStack[mCurrentStack - 1]->Release();
	mCurrentStack -= 2;

	return PushObject(r);
}

int runtimeContext::OnInst_div(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2)
	{
		SCRIPT_TRACE("OnInst_div: Stack outof range\n");
		return -1;
	}

	runtimeObjectBase *r = mRuntimeStack[mCurrentStack - 2]->Div(mRuntimeStack[mCurrentStack - 1]);
	if (!r) {
		SCRIPT_TRACE("Invalid div operation between type [%u] and [%u].\n", 
		mRuntimeStack[mCurrentStack - 2]->GetObjectTypeId(),
		mRuntimeStack[mCurrentStack - 1]->GetObjectTypeId());
		return -1;
	}

	mRuntimeStack[mCurrentStack - 2]->Release();
	mRuntimeStack[mCurrentStack - 1]->Release();
	mRuntimeStack[mCurrentStack - 2] = r;
	r->AddRef();
	mCurrentStack--;
	return 0;
}

int runtimeContext::OnInst_createInt(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	bool isConst = !!inst->extCode;
	if (PushObject(baseTypeObject::CreateBaseTypeObject<intObject>(isConst)) < 0)
		return -1;
	static_cast<intObject*>(mRuntimeStack[mCurrentStack - 1])->mVal = *reinterpret_cast<int*>(moreData);
	return 0;
}

int runtimeContext::OnInst_createFloat(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	bool isConst = !!inst->extCode;
	if (PushObject(baseTypeObject::CreateBaseTypeObject<floatObject>(isConst)) < 0)
		return -1;
	static_cast<floatObject*>(mRuntimeStack[mCurrentStack - 1])->mVal = *reinterpret_cast<float*>(moreData);
	return 0;
}

int runtimeContext::OnInst_createString(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	bool isConst = !!inst->extCode;
	if (PushObject(baseTypeObject::CreateBaseTypeObject<stringObject>(isConst)) < 0)
		return -1;
	mCompileResult->GetStringData()->GetString(*reinterpret_cast<uint32_t*>(moreData), *static_cast<stringObject*>(mRuntimeStack[mCurrentStack - 1])->mVal);
	return 0;
}

int runtimeContext::OnInst_createArray(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (PushObject(baseTypeObject::CreateBaseTypeObject<arrayObject>(false)) < 0)
		return -1;
	return 0;
}

int runtimeContext::OnInst_end(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	return -100;
}

int runtimeContext::OnInst_pushStackFrame(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mStackFrameSize == mStackFrame.size())
	{
		SCRIPT_TRACE("OnInst_pushStackFrame栈帧已满\n");
		return -1;
	}
	mStackFrame[mStackFrameSize++] = mCurrentStack;
	return 0;
}

int runtimeContext::OnInst_popStackFrame(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (!mStackFrameSize)
	{
		SCRIPT_TRACE("OnInst_popStackFrame时栈帧队列为空\n");
		return -1;
	}
	uint32_t oldStack = mCurrentStack;
	mCurrentStack = mStackFrame[--mStackFrameSize];
	for (uint32_t i = mCurrentStack; i < oldStack; i ++)
	{
		mRuntimeStack[i]->Release();
	}
	return 0;
}

int runtimeContext::OnInst_copyAtFrame(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	assert(moreSize == 4);
	uint32_t index = *reinterpret_cast<uint32_t*>(moreData);
	uint32_t level = inst->data;

	if (level >= mStackFrameSize)
	{
		SCRIPT_TRACE("OnInst_copyAtFrame访问栈帧越界.\n");
		return -1;
	}

	uint32_t stackPos = mStackFrame[mStackFrameSize - level - 1] + index;
	if (stackPos >= mCurrentStack)
	{
		SCRIPT_TRACE("OnInst_copyAtFrame企图访问不存在数据的栈空间.\n");
		return -1;
	}
	
	return PushObject(mRuntimeStack[stackPos]);
}

int runtimeContext::OnInst_setVal(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2)
	{
		SCRIPT_TRACE("OnInst_setVal: Stack outof range\n");
		return -1;
	}

	runtimeObjectBase *r = mRuntimeStack[mCurrentStack - 2]->SetValue(mRuntimeStack[mCurrentStack - 1]);
	if (!r) {
		SCRIPT_TRACE("Invalid setval operation between type [%u] and [%u].\n", 
		mRuntimeStack[mCurrentStack - 2]->GetObjectTypeId(),
		mRuntimeStack[mCurrentStack - 1]->GetObjectTypeId());
		return -1;
	}

	// 栈顶的目标值对象被删除，而mCurrentStack-2就是赋值的那个对象，无需处理
	mRuntimeStack[mCurrentStack - 1]->Release();
	mCurrentStack--;
	return 0;
}

int runtimeContext::PushObject(runtimeObjectBase *obj)
{
	if (mRuntimeStack.size() == mCurrentStack)
	{
		SCRIPT_TRACE("堆栈上溢出\n");
		return -1;
	}
	obj->AddRef();
	mRuntimeStack[mCurrentStack++] = obj;
	return 0;
}

void runtimeContext::SetCompareResult(bool r)
{
	mRuntimeStack[mCurrentStack - 2]->Release();
	mRuntimeStack[mCurrentStack - 1]->Release();
	mCurrentStack -= 2;
	PushObject(new ObjectModule<intObject>);
	static_cast<intObject*>(mRuntimeStack[mCurrentStack - 1])->mVal = r ? 1 : 0;
}

int runtimeContext::OnInst_equal(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("OnInst_equal: Stack out of range.\n");
		return -1;
	}
	bool r = mRuntimeStack[mCurrentStack - 2]->isEqual(mRuntimeStack[mCurrentStack - 1]);
	SetCompareResult(r);
	return 0;
}

int runtimeContext::OnInst_notEqual(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("OnInst_notEqual: Stack out of range.\n");
		return -1;
	}
	bool r = !mRuntimeStack[mCurrentStack - 2]->isEqual(mRuntimeStack[mCurrentStack - 1]);
	SetCompareResult(r);
	return 0;
}

int runtimeContext::OnInst_greater(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("OnInst_greater: Stack out of range.\n");
		return -1;
	}
	bool r = mRuntimeStack[mCurrentStack - 2]->isGreaterThan(mRuntimeStack[mCurrentStack - 1]);
	SetCompareResult(r);
	return 0;
}

int runtimeContext::OnInst_less(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("OnInst_less: Stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *r1 = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *r2 = mRuntimeStack[mCurrentStack - 1];
	bool r = !(r1->isEqual(r2) || r1->isGreaterThan(r2));
	SetCompareResult(r);
	return 0;
}

int runtimeContext::OnInst_greaterEqual(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("OnInst_greaterEqual: Stack out of range.\n");
		return -1;
	}
	bool r = (mRuntimeStack[mCurrentStack - 2]->isGreaterThan(mRuntimeStack[mCurrentStack - 1]) ||
		mRuntimeStack[mCurrentStack - 2]->isEqual(mRuntimeStack[mCurrentStack - 1]));
	SetCompareResult(r);
	return r;
}

int runtimeContext::OnInst_lessEqual(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("OnInst_greaterEqual: Stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *r1 = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *r2 = mRuntimeStack[mCurrentStack - 1];
	bool r = !r1->isGreaterThan(r2);
	SetCompareResult(r);
	return r;
}

int runtimeContext::OnInst_jump(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	assert(moreSize);
	uint32_t targetPos = *reinterpret_cast<uint32_t*>(moreData);
	compiler::CompileResult::ScriptCode &code = mCompileResult->GetCode();
	if (targetPos > code.size())
	{
		SCRIPT_TRACE("OnInst_jz: Jump over the code area.\n");
		return -1;
	}
	mPC = &code[targetPos];
	return 1;
}

int runtimeContext::OnInst_jnz(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 1) {
		SCRIPT_TRACE("OnInst_jnz: Stack out of range.\n");
		return -1;
	}

	assert(moreSize);
	uint32_t targetPos = *reinterpret_cast<uint32_t*>(moreData);
	compiler::CompileResult::ScriptCode &code = mCompileResult->GetCode();
	if (targetPos > code.size())
	{
		SCRIPT_TRACE("OnInst_jnz: Jump over the code area.\n");
		return -1;
	}

	bool shouldJump = false;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 1];

	// 判定
	uint32_t theType = o->GetObjectTypeId();
	switch(theType)
	{
	case DT_int8:
		if (static_cast<charObject*>(o)->mVal)
			shouldJump = true;
		break;

	case DT_uint8:
		if (static_cast<byteObject*>(o)->mVal)
			shouldJump = true;
		break;

	case DT_int16:
		if (static_cast<shortObject*>(o)->mVal)
			shouldJump = true;
		break;

	case DT_uint16:
		if (static_cast<ushortObject*>(o)->mVal)
			shouldJump = true;
		break;

	case DT_int32:
		if (static_cast<intObject*>(o)->mVal)
			shouldJump = true;
		break;

	case DT_uint32:
		if (static_cast<uintObject*>(o)->mVal)
			shouldJump = true;
		break;

	case DT_float:
		if (fabs(static_cast<floatObject*>(o)->mVal) > 0.0000001)
			shouldJump = true;
		break;

	case DT_double:
		if (fabs(static_cast<doubleObject*>(o)->mVal) > 0.0000001)
			shouldJump = true;
		break;

	default:
		break;
	}

	o->Release();
	mCurrentStack--;
	if (shouldJump)
	{
		mPC = &code[targetPos];
		return 1;
	}

	return 0;
}

int runtimeContext::OnInst_jz(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 1) {
		SCRIPT_TRACE("OnInst_jz: Stack out of range.\n");
		return -1;
	}
	assert(moreSize);
	uint32_t targetPos = *reinterpret_cast<uint32_t*>(moreData);
	compiler::CompileResult::ScriptCode &code = mCompileResult->GetCode();
	if (targetPos > code.size())
	{
		SCRIPT_TRACE("OnInst_jz: Jump over the code area.\n");
		return -1;
	}

	bool shouldJump = true;
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 1];

	// 判定
	uint32_t theType = o->GetObjectTypeId();
	switch(theType)
	{
	case DT_int8:
		if (static_cast<charObject*>(o)->mVal)
			shouldJump = false;
		break;

	case DT_uint8:
		if (static_cast<byteObject*>(o)->mVal)
			shouldJump = false;
		break;

	case DT_int16:
		if (static_cast<shortObject*>(o)->mVal)
			shouldJump = false;
		break;

	case DT_uint16:
		if (static_cast<ushortObject*>(o)->mVal)
			shouldJump = false;
		break;

	case DT_int32:
		if (static_cast<intObject*>(o)->mVal)
			shouldJump = false;
		break;

	case DT_uint32:
		if (static_cast<uintObject*>(o)->mVal)
			shouldJump = false;
		break;

	case DT_float:
		if (fabs(static_cast<floatObject*>(o)->mVal) > 0.0000001)
			shouldJump = false;
		break;

	case DT_double:
		if (fabs(static_cast<doubleObject*>(o)->mVal) > 0.0000001)
			shouldJump = false;
		break;

	default:
		break;
	}

	o->Release();
	mCurrentStack--;
	if (shouldJump)
	{
		mPC = &code[targetPos];
		return 1;
	}

	return 0;
}

int runtimeContext::OnInst_debug1(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	// 目前什么都不作
	return 0;
}

template <typename T>
class OperatorBitwiseAnd
{
public:
	inline T operator()(T t1, T t2) { return t1 & t2; }
};

template <typename T>
class OperatorBitwiseOr
{
public:
	inline T operator()(T t1, T t2) { return t1 | t2; }
};

template <typename T>
class OperatorBitwiseXor
{
public:
	inline T operator()(T t1, T t2) { return t1 ^ t2; }
};

template <typename T>
class OperatorBitwiseNot
{
public:
	inline T operator()(T t) { return ~t; }
};

int runtimeContext::OnInst_bitwiseAnd(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseAnd: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *p = mRuntimeStack[mCurrentStack - 1];
	if (!isIntegerType(p)) {
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseAnd: invalid p typeid.\n");
		return -1;
	}
	return bitwiseOperInner<OperatorBitwiseAnd>(o, p);
}

int runtimeContext::OnInst_bitwiseOr(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseOr: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *p = mRuntimeStack[mCurrentStack - 1];
	if (!isIntegerType(p)) {
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseOr: invalid p typeid.\n");
		return -1;
	}
	return bitwiseOperInner<OperatorBitwiseOr>(o, p);
}

int runtimeContext::OnInst_bitwiseXOR(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseXOR: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *p = mRuntimeStack[mCurrentStack - 1];
	if (!isIntegerType(p)) {
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseXOR: invalid p typeid.\n");
		return -1;
	}
	return bitwiseOperInner<OperatorBitwiseXor>(o, p);
}

int runtimeContext::OnInst_bitwiseNot(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 1) {
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseNot: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 1];
	switch (o->GetObjectTypeId())
	{
	case DT_int8:
		OneOperatorBitwiseOperation<charObject,OperatorBitwiseNot<int8_t> >(o);
		break;

	case DT_int16:
		OneOperatorBitwiseOperation<shortObject,OperatorBitwiseNot<int16_t> >(o);
		break;

	case DT_int32:
		OneOperatorBitwiseOperation<intObject,OperatorBitwiseNot<int32_t> >(o);
		break;

	case DT_uint8:
		OneOperatorBitwiseOperation<byteObject,OperatorBitwiseNot<uint8_t> >(o);
		break;

	case DT_uint16:
		OneOperatorBitwiseOperation<ushortObject,OperatorBitwiseNot<uint16_t> >(o);
		break;

	case DT_uint32:
		OneOperatorBitwiseOperation<uintObject,OperatorBitwiseNot<uint32> >(o);
		break;

	default:
		SCRIPT_TRACE("runtimeContext::OnInst_bitwiseNot: invalid bitwise operator.\n");
		break;
	}
	return 0;
}

int runtimeContext::OnInst_logicAnd(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("runtimeContext::OnInst_logicAnd: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *p = mRuntimeStack[mCurrentStack - 1];
	runtime::intObject *r = new runtime::ObjectModule<intObject>;
	r->mVal = !isZero(o) && !isZero(p);
	o->Release();
	p->Release();
	mCurrentStack -= 2;
	return PushObject(r);
}

int runtimeContext::OnInst_logicOr(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 2) {
		SCRIPT_TRACE("runtimeContext::OnInst_logicOr: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 2];
	runtimeObjectBase *p = mRuntimeStack[mCurrentStack - 1];
	runtime::intObject *r = new runtime::ObjectModule<intObject>;
	r->mVal = !isZero(o) || !isZero(p);
	o->Release();
	p->Release();
	mCurrentStack -= 2;
	return PushObject(r);
}

int runtimeContext::OnInst_logicNot(Instruction *inst, uint8_t *moreData, uint32_t moreSize)
{
	if (mCurrentStack < 1) {
		SCRIPT_TRACE("runtimeContext::OnInst_logicNot: stack out of range.\n");
		return -1;
	}
	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - 1];
	runtime::intObject *r = new runtime::ObjectModule<intObject>;
	r->mVal = !!isZero(o);
	o->Release();
	mCurrentStack--;
	return PushObject(r);
}

const runtimeContext::InstructionEntry runtimeContext::mIES[256] = 
{
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 1
	{ &runtimeContext::OnInst_add, 0, },
	{ &runtimeContext::OnInst_sub, 0, },
	{ &runtimeContext::OnInst_mul, 0, },
	{ &runtimeContext::OnInst_div, 0, },
	{ &runtimeContext::OnInst_mod, 0, },
	{ &runtimeContext::OnInst_logicAnd, 0, },
	{ &runtimeContext::OnInst_logicOr, 0, },
	{ &runtimeContext::OnInst_logicNot, 0, },
	{ &runtimeContext::OnInst_bitwiseAnd, 0, },
	{ &runtimeContext::OnInst_bitwiseOr, 0, },

	// 11
	{ &runtimeContext::OnInst_bitwiseXOR, 0, },
	{ &runtimeContext::OnInst_bitwiseNot, 0, },
	{ &runtimeContext::OnInst_setVal, 0, },
	{ &runtimeContext::OnInst_equal, 0, },
	{ &runtimeContext::OnInst_notEqual, 0, },
	{ &runtimeContext::OnInst_greater, 0, },
	{ &runtimeContext::OnInst_less, 0, },
	{ &runtimeContext::OnInst_greaterEqual, 0, },
	{ &runtimeContext::OnInst_lessEqual, 0, },
	{ &runtimeContext::OnInst_jz, 4, },

	// 21
	{ &runtimeContext::OnInst_jump, 4, },
	{ &runtimeContext::OnInst_getMember, 4, },
	{ &runtimeContext::OnInst_doCall, 0, },
	{ &runtimeContext::OnInst_getIndex, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInst_createInt, 4, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 31
	{ &runtimeContext::OnInst_createFloat, 4, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInst_createString, 4, },
	{ &runtimeContext::OnInst_createArray, 4, },
	{ &runtimeContext::OnInst_push, 0, },
	{ &runtimeContext::OnInst_pop, 0, },
	{ &runtimeContext::OnInst_copyAtFrame, 4, },
	{ &runtimeContext::OnInst_pushStackFrame, 0, },
	{ &runtimeContext::OnInst_popStackFrame, 0, },
	{ &runtimeContext::OnInst_end, 0, },

	// 41
	{ &runtimeContext::OnInst_setParamCount, 0, },
	{ &runtimeContext::OnInst_debug1, 4, },
	{ &runtimeContext::OnInst_jnz, 4, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 51
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 61
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 71
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 81
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 91
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 101
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 111
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 121
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 131
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 141
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 151
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 161
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 171
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 181
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 191
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 201
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 211
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 221
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 231
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 241
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },

	// 251
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
	{ &runtimeContext::OnInvalidInstruction, 0, },
};

void runtimeContext::RunInner()
{
	for (;;)
	{
		if (mPC >= mPCEnd)
			break;

		Instruction *inst = reinterpret_cast<Instruction*>(mPC++);
		OnInstruction onInst = mIES[inst->code].inst;

		int r;
		uint32_t s = mIES[inst->code].moreSize;
		// 返回<0的值表示退出
		// 返回=0表示指令指针需要按需调整
		// 返回>0的值表示可能经过了跳转，无需调整指令指针了
		if ((r = (this->*onInst)(inst, reinterpret_cast<uint8_t*>(mPC), s)) < 0)
			break;
		if (s && !r)
			mPC = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(mPC)+s);
	}
}

int runtimeContext::Execute(void *code, compiler::CompileResult *compileResult)
{
	scriptAPI::ScriptCompiler::CompileCode *theCode = 
		reinterpret_cast<scriptAPI::ScriptCompiler::CompileCode*>(code);

	mCompileResult = compileResult;
	mPC = theCode->code;
	mPCEnd = mPC + theCode->sizeInUint32;

	RunInner();

	return 0;
}

int runtimeContext::Execute(compiler::CompileResult *compileResult)
{
	mCompileResult = compileResult;
	mPC = &mCompileResult->GetCode()[0];
	mPCEnd = mPC + mCompileResult->GetCode().size();
	
	uint32_t stackPosition = mCurrentStack;

	RunInner();

	// 恢复堆栈，以便下一次运行
	for (uint32_t x = mCurrentStack - 1; x >= stackPosition; x--)
		mRuntimeStack[x]->Release();
	mCurrentStack = stackPosition;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
