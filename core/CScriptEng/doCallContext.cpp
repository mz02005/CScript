#include "stdafx.h"
#include "vm.h"
#include "CScriptEng.h"
#include "arrayType.h"

using namespace runtime;

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

runtimeObjectBase* runtimeContext::GetObject(uint32_t i)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetObject: out of range\n");

	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + 1];
	// 需要使用者Release来平衡引用计数
	o->AddRef();

	return o;
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

runtimeObjectBase* runtimeContext::GetObjectOfArrayParam(uint32_t i, uint32_t e)
{
	if (i >= mParamCount || i >= mCurrentStack)
		throw std::exception("runtimeContext::GetStringElemOfArrayParam: out of range\n");

	runtimeObjectBase *o = mRuntimeStack[mCurrentStack - mParamCount + i];

	if (o->GetObjectTypeId() != DT_array)
		throw std::bad_cast("runtimeContext::GetStringElemOfArrayParam: not array\n");

	arrayObject *a = static_cast<arrayObject*>(o);
	if (e >= a->mData->size())
		throw std::out_of_range("runtimeContext::GetStringElemOfArrayParam");

	runtimeObjectBase *sub = a->mData->operator[](e);
	sub->AddRef();

	return sub;
}
