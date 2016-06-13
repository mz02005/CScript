#include "stdAfx.h"
#include "rtlib.h"
#include "vm.h"
#include "arrayType.h"
#include <math.h>
#include <time.h>

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

sleepObj::sleepObj()
{
}

uint32_t sleepObj::GetObjectTypeId() const
{
	return DT_UserTypeBegin;
}

runtimeObjectBase* sleepObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 1)
		return NULL;

#if defined(PLATFORM_WINDOWS)
	::Sleep(context->GetUint32Param(0));
#else
	uint32_t milisecond = context->GetUint32Param(0);
	timespec ts =
	{
		(time_t)(milisecond / 1000),
		(milisecond % 1000) * 1000000,
	};
	nanosleep(&ts, NULL);
#endif

	return new ObjectModule<runtime::baseObjDefault>();
}



bool rtLibHelper::RegistObjNames(compiler::FunctionStatement *sb)
{
	sb->RegistNameInContainer("sleep", -1);
	sb->RegistNameInContainer("print", -1);
	sb->RegistNameInContainer("println", -1);
	sb->RegistNameInContainer("rand", -1);
	sb->RegistNameInContainer("srand", -1);
	sb->RegistNameInContainer("sin", -1);
	sb->RegistNameInContainer("pow", -1);
	sb->RegistNameInContainer("time", -1);
	return true;
}

bool rtLibHelper::RegistRuntimeObjs(runtimeContext *context)
{
	context->PushObject(new runtime::ObjectModule<runtime::sleepObj>);

	// printºÍprintln
	context->PushObject(new runtime::ObjectModule<runtime::printObj>);
	runtime::printObj *println = new runtime::ObjectModule<runtime::printObj>;
	println->SetIsPrintLine(true);
	context->PushObject(println);

	context->PushObject(new runtime::ObjectModule<runtime::randObj>);
	context->PushObject(new runtime::ObjectModule<runtime::srandObj>);

	context->PushObject(new runtime::ObjectModule<runtime::sinObj>);
	context->PushObject(new runtime::ObjectModule<runtime::powfObj>);

	context->PushObject(new runtime::ObjectModule<runtime::timeObj>);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

printObj::printObj()
	: mPrintLine(false)
{
}

void printObj::SetIsPrintLine(bool isPrintLine)
{
	mPrintLine = isPrintLine;
}

runtimeObjectBase* printObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 1)
		return NULL;
	runtime::stringObject *s = context->GetParam(0)->toString();
	if (s)
	{
		printf("%s%s", s->mVal->c_str(), mPrintLine ? "\n" : "");
#if defined(PLATFORM_WINDOWS)
		OutputDebugStringA(s->mVal->c_str());
		if (mPrintLine)
			OutputDebugStringA("\n");
#endif
		return s;
	}
	return new runtime::ObjectModule<runtime::baseObjDefault>();
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* randObj::doCall(runtime::doCallContext *context)
{
	runtime::floatObject *f = new runtime::ObjectModule<runtime::floatObject>;
	f->mVal = ((floatObject::InnerDataType)(rand()%RAND_MAX)) / RAND_MAX;
	return f;
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* srandObj::doCall(runtime::doCallContext *context)
{
	switch (context->GetParamCount())
	{
	case 0:
		srand(unsigned(time(NULL)));
		break;

	case 1:
		srand(context->GetUint32Param(0));
		break;

	default:
		printf("srand fail\n");
		return NULL;
	}
	return this;
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* sinObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 1)
		return NULL;

	runtime::floatObject *f = new runtime::ObjectModule<runtime::floatObject>;

	f->mVal = static_cast<float>(sin(context->GetDoubleParam(0)));
	return f;
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* powfObj::doCall(runtime::doCallContext *context)
{
	runtime::floatObject *f = new runtime::ObjectModule<runtime::floatObject>;
	f->mVal = 1.f;

	if (context->GetParamCount() != 2)
		return f;

	f->mVal = (float)pow(context->GetDoubleParam(0), context->GetDoubleParam(1));
	return f;
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* timeObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 0)
	{
		SCRIPT_TRACE("timeObj::doCall: Invalid param count.\n");
		return NULL;
	}

	time_t t = ::time(NULL);
	struct tm r;
#if defined(PLATFORM_WINDOWS)
	if (localtime_s(&r, &t))
		return NULL;
#else
	struct tm *x = localtime(&t);
	if (!x)
		return NULL;
	r = *x;
#endif

	runtime::arrayObject *a = new runtime::ObjectModule<runtime::arrayObject>;
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_year + 1900));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_mon + 1));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_mday));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_hour));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_min));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_sec));

	return a;
}

///////////////////////////////////////////////////////////////////////////////
