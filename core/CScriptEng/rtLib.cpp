#include "stdAfx.h"
#include "rtlib.h"
#include "vm.h"
#include "arrayType.h"
#include <math.h>
#include <time.h>
#include "notstd/zipWrapper.h"
#include "objType.h"

namespace runtime {
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
		sb->RegistNameInContainer("null", -1);
		sb->RegistNameInContainer("sleep", -1);
		sb->RegistNameInContainer("print", -1);
		sb->RegistNameInContainer("println", -1);
		sb->RegistNameInContainer("rand", -1);
		sb->RegistNameInContainer("srand", -1);
		sb->RegistNameInContainer("sin", -1);
		sb->RegistNameInContainer("pow", -1);
		sb->RegistNameInContainer("time", -1);
		sb->RegistNameInContainer("system", -1);
		sb->RegistNameInContainer("unzipFile", -1);
		sb->RegistNameInContainer("zipFile", -1);
		sb->RegistNameInContainer("csOpenFile", -1);
		return true;
	}

	bool rtLibHelper::RegistRuntimeObjs(runtimeContext *context)
	{
		context->PushObject(NullTypeObject::CreateNullTypeObject());
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

		context->PushObject(new runtime::ObjectModule<runtime::systemCallObject>);

		context->PushObject(new runtime::ObjectModule<runtime::unzipFileObj>);
		context->PushObject(new runtime::ObjectModule<runtime::zipFilesInDirectoryObj>);

		context->PushObject(new runtime::ObjectModule<runtime::csOpenFile>);

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
		f->mVal = ((floatObject::InnerDataType)(rand() % RAND_MAX)) / RAND_MAX;
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

	runtimeObjectBase* systemCallObject::doCall(runtime::doCallContext *context)
	{
		runtime::intObject *r = new runtime::ObjectModule<runtime::intObject>;
		r->mVal = -1;

		const char *cmd;
		if (context->GetParamCount() < 1
			|| (cmd = context->GetStringParam(0)) == nullptr)
			return r;

		r->mVal = ::system(cmd);
		return r;
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* unzipFileObj::doCall(runtime::doCallContext *context)
	{
		runtime::intObject *r = new runtime::ObjectModule<runtime::intObject>;
		r->mVal = -notstd::zlibHelper::E_PARAMERROR;

		notstd::zlibHelper::UnzipParam up;
		memset(&up, 0, sizeof(up));

		if (context->GetParamCount() < 2)
			return r;
		if (context->GetParamCount() >= 3)
		{
			uint32_t ap = context->GetArrayParamElemCount(2);
			if (ap > 0)
			{
				up.showVerb = context->GetInt32ElemOfArrayParam(2, 0);
			}
		}
		std::string dir, zipFile;
		const char *p1 = context->GetStringParam(0);
		const char *p2 = context->GetStringParam(1);
		if (!p1 || !p2)
		{
			r->mVal = -notstd::zlibHelper::E_PARAMERROR;
			return r;
		}
		dir = p1; zipFile = p2;
		r->mVal = notstd::zlibHelper::unzipToDirectory(dir, zipFile, &up);
		return r;
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* zipFilesInDirectoryObj::doCall(runtime::doCallContext *context)
	{
		runtime::intObject *r = new runtime::ObjectModule<intObject>;
		r->mVal = -notstd::zlibHelper::E_PARAMERROR;

		if (context->GetParamCount() < 2)
			return r;

		const char *p1 = context->GetStringParam(0);
		const char *p2 = context->GetStringParam(1);
		if (!p1 || !p2)
		{
			r->mVal = -notstd::zlibHelper::E_PARAMERROR;
			return r;
		}
		std::string dir = p2, zipFilePath = p1;
		r->mVal = notstd::zlibHelper::zipDirectoryToFile(
			zipFilePath, dir);

		return r;
	}

	///////////////////////////////////////////////////////////////////////////////

	class FileObject;

	class WriteFileObject : public runtime::baseObjDefault
	{
		friend class FileObject;

	private:
		FileObject *mFileObject;

	public:
		virtual runtime::runtimeObjectBase* doCall(doCallContext *context);
	};

	class CloseFileObject : public runtime::baseObjDefault
	{
		friend class FileObject;

	private:
		FileObject *mFileObject;

	public:
		virtual runtime::runtimeObjectBase* doCall(doCallContext *context);
	};

	class FileObject : public runtime::baseObjDefault
	{
		friend class csOpenFile;

	private:
		FILE *mFile;

	public:
		FileObject()
			: mFile(nullptr)
		{
		}
		virtual runtimeObjectBase* GetMember(const char *memName)
		{
			if (!strcmp(memName, "WriteFile"))
			{
				WriteFileObject *o = new runtime::ContainModule<WriteFileObject>(this);
				o->mFileObject = this;
				return o;
			}
			if (!strcmp(memName, "Close"))
			{
				CloseFileObject *o = new runtime::ContainModule<CloseFileObject>(this);
				o->mFileObject = this;
				return o;
			}
			return runtime::baseObjDefault::GetMember(memName);
		}
		FILE* getFileHandle() { return mFile; }
	};

	runtimeObjectBase* csOpenFile::doCall(runtime::doCallContext *context)
	{
		if (context->GetParamCount() < 1)
			return NullTypeObject::CreateNullTypeObject();

		const char *fn = context->GetStringParam(0);
		if (!fn)
			return NullTypeObject::CreateNullTypeObject();

		std::string of = "wb";
		if (context->GetParamCount() == 2)
		{
			const char *openFlag = context->GetStringParam(1);
			if (!openFlag)
				return NullTypeObject::CreateNullTypeObject();
			of = openFlag;
		}

		std::string dir = fn;
		std::string::size_type f = dir.rfind(notstd::zlibHelper::mSplash[0]);
		if (f != dir.npos)
			dir.erase(f);
		notstd::zlibHelper::CreateDirectoryR(dir);

		FILE *file = ::fopen(fn, of.c_str());
		if (!file)
			return NullTypeObject::CreateNullTypeObject();

		FileObject *fo = new runtime::ObjectModule<FileObject>;
		fo->mFile = file;
		return fo;
	}

	///////////////////////////////////////////////////////////////////////////////

	runtime::runtimeObjectBase* WriteFileObject::doCall(doCallContext *context)
	{
		const char *s;
		if (context->GetParamCount() != 1
			|| ((s = context->GetStringParam(0)) == nullptr))
			return nullptr;
		runtime::intObject *r = new runtime::ObjectModule<runtime::intObject>;
		r->mVal = fwrite(s, 1, strlen(s), mFileObject->getFileHandle());
		return r;
	}

	runtime::runtimeObjectBase* CloseFileObject::doCall(doCallContext *context)
	{
		runtime::intObject *r = new runtime::ObjectModule<runtime::intObject>;
		r->mVal = fclose(mFileObject->getFileHandle());
		return r;
	}

	///////////////////////////////////////////////////////////////////////////////
}
