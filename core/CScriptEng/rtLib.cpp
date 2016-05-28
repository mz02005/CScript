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



bool rtLibHelper::RegistObjNames(compiler::FunctionStatement *sb)
{
	sb->RegistNameInContainer("sleep", -1);
	sb->RegistNameInContainer("print", -1);
	sb->RegistNameInContainer("println", -1);
	sb->RegistNameInContainer("rand", -1);
	sb->RegistNameInContainer("srand", -1);
	sb->RegistNameInContainer("sin", -1);
	sb->RegistNameInContainer("time", -1);
	sb->RegistNameInContainer("substr", -1);
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

	context->PushObject(new runtime::ObjectModule<runtime::timeObj>);

	context->PushObject(new runtime::ObjectModule<runtime::substrObj>);

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
		printf("%s%s", s->mVal->c_str(), mPrintLine ? "\n" : "");
#if defined(WIN32) && (defined(DEBUG) || defined(_DEBUG))
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

///////////////////////////////////////////////////////////////////////////////

runtimeObjectBase* substrObj::doCall(runtime::doCallContext *context)
{
	uint32_t c = context->GetParamCount();
	std::string::size_type from = 0, size = -1;
	stringObject *r = new runtime::ObjectModule<stringObject>;
	std::string s;
	if (c < 2)
		return r;
	if (c >= 1)
	{
		s = context->GetStringParam(0);
	}
	if (c >= 2)
	{
		from = context->GetUint32Param(1);
	}
	if (c >= 3)
	{
		size = context->GetUint32Param(2);
	}
	*r->mVal = StringHelper::Mid(s, from, size);
	return r;
}
