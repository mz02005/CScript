#include "stdAfx.h"
#include "rtlib.h"
#include "vm.h"
#include "arrayType.h"

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
		return nullptr;

	::Sleep(context->GetUint32Param(0));

	return new ObjectModule<runtime::baseObjDefault>();
}



bool rtLibHelper::RegistObjNames(compiler::StatementBlock *sb)
{
	sb->PushName("sleep");
	sb->PushName("print");
	sb->PushName("println");
	sb->PushName("rand");
	sb->PushName("srand");
	sb->PushName("sin");
	sb->PushName("time");
	return true;
}

bool rtLibHelper::RegistRuntimeObjs(runtimeContext *context)
{
	context->PushObject(new runtime::ObjectModule<runtime::sleepObj>);

	// print��println
	context->PushObject(new runtime::ObjectModule<runtime::printObj>);
	runtime::printObj *println = new runtime::ObjectModule<runtime::printObj>;
	println->SetIsPrintLine(true);
	context->PushObject(println);

	context->PushObject(new runtime::ObjectModule<runtime::randObj>);
	context->PushObject(new runtime::ObjectModule<runtime::srandObj>);

	context->PushObject(new runtime::ObjectModule<runtime::sinObj>);

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
	runtime::stringObject *s = context->GetParam(0)->toString();
	if (s)
	{
		printf("%s", s->mVal->c_str());
		if (mPrintLine)
			printf("\n");
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
		return nullptr;
	}
	return this;
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* sinObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 1)
		return nullptr;

	runtime::floatObject *f = new runtime::ObjectModule<runtime::floatObject>;

	f->mVal = static_cast<float>(sin(context->GetDoubleParam(0)));
	return f;
}

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* timeObj::doCall(runtime::doCallContext *context)
{
	if (context->GetParamCount() != 0)
	{
		SCRIPT_TRACE("timeObj::doCall: Invalid param count.\n");
		return nullptr;
	}

	time_t t = ::time(nullptr);
	struct tm r;
	if (localtime_s(&r, &t))
		return nullptr;

	runtime::arrayObject *a = new runtime::ObjectModule<runtime::arrayObject>;
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_year + 1900));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_mon + 1));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_mday));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_hour));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_min));
	a->AddSub(runtime::intObject::CreateIntObject(r.tm_sec));

	return a;
}
