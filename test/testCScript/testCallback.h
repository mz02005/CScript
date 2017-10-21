#pragma once
#include "CScriptEng/CScriptEng.h"

#define TESTCALLBACKNAME "TestFunctionCallback"

class TestCallback : public runtime::runtimeObjectBase
{
public:
	TestCallback()
	{
	}

	virtual uint32_t GetObjectTypeId() const
	{
		return runtime::DT_UserTypeBegin;
	}

	virtual runtimeObjectBase* Add(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Div(const runtimeObjectBase *obj){ return NULL; }

	// =二元运算
	virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj) { return NULL; }

	// 处理.操作符（一元的）
	virtual runtimeObjectBase* GetMember(const char *memName) { return NULL; }

	// docall（函数调用一元运算）
	virtual runtimeObjectBase* doCall(runtime::doCallContext *context)
	{
		if (context->GetParamCount() != 1)
			return this;

		if (context->GetParam(0)->GetObjectTypeId() != runtime::DT_function)
			return this;

		runtime::FunctionObject *f = static_cast<runtime::FunctionObject*>(context->GetParam(0));
		if (f->GetDesc()->paramCount != 1)
		{
			printf("Invalid param count for function %s\n",
				f->GetFuncName().c_str());
			return this;
		}
		// 调用回调函数，它具有1个参数
		context->SetParamCount(1);
		runtime::intObject *iVal = new runtime::ObjectModule<runtime::intObject>;
		iVal->mVal = 100;
		context->PushObjectToStack(iVal);
		runtime::runtimeObjectBase *r = f->doCall(context);
		// 之前调用TestFunctionCallback时，该函数具有一个参数，恢复这个设置
		context->SetParamCount(1);
		return r;
	}

	// getindex（索引访问一元运算）
	virtual runtimeObjectBase* getIndex(int i) { return NULL; }
	// 对象转化为字符串
	virtual runtime::stringObject* toString() {
		runtime::stringObject *s = new runtime::ObjectModule<runtime::stringObject>;
		*s->mVal = "TestCallbackFunction";
		return s;
	}

	// 比较
	virtual bool isGreaterThan(runtimeObjectBase *obj) { return false; }

	virtual bool isEqual(runtimeObjectBase *obj) { return false; }
};
