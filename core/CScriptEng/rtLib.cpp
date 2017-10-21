#include "stdafx.h"
#include "rtlib.h"
#include "vm.h"
#include "arrayType.h"
#include <math.h>
#include <time.h>
#include "notstd/zipWrapper.h"
#include "objType.h"
#include "svnInfo.h"
#include "BufferObject.h"
#include <algorithm>
#include "netLib.h"

#ifdef min
#undef min
#endif

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



	bool rtLibHelper::RegistCScriptRuntimeLib(compiler::SimpleCScriptEngContext *context)
	{
		printObj *println = new ObjectModule<printObj>;
		println->SetIsPrintLine(true);

		scriptAPI::ScriptLibReg slrs[] =
		{
			{ "null", NullTypeObject::CreateNullTypeObject(), },
			{ "getversion", new ObjectModule<getVersionObj>, },
			{ "CreateArray", new runtime::ObjectModule<runtime::CreateArrayObj>, },
			{ "sleep", new ObjectModule<sleepObj>, },
			{ "print", new ObjectModule<printObj>, },
			{ "println", println, },
			{ "rand", new ObjectModule<randObj>, },
			{ "srand", new ObjectModule<srandObj>, },
			{ "sin", new ObjectModule<sinObj>, },
			{ "pow", new ObjectModule<powfObj>, },
			{ "time", new ObjectModule<timeObj>, },
			{ "system", new ObjectModule<systemCallObject>, },
			{ "ParseInt32", new ObjectModule<ParseInt32Obj>, },
			{ "ParseUint32", new ObjectModule<ParseUint32Obj>, },
			{ "unzipFile", new ObjectModule<unzipFileObj>, },
			{ "zipFile", new ObjectModule<zipFilesInDirectoryObj>, },
			{ "csOpenFile", new ObjectModule<csOpenFile>, },
			{ "DeleteFile", new ObjectModule<DeleteFileObj>, },
			{ "CreateBuffer", new ObjectModule<CreateBufferObj>, },
			{ "GetCurrentTick", new ObjectModule<GetTickCountObj>, },
			{ "tcpConnect", new ObjectModule<tcpConnectObj>, },
			{ "AddressToString", new ObjectModule<inet_ntoaObj>, },
			{ "AddressFromHostname", new ObjectModule<AddressFromHostnameObj>, },

			{ "NetworkByteOrderToHostByteOrderS", 
			NetworkByteOrderToHostByteOrderObj::Create(NetworkByteOrderToHostByteOrderObj::BO_UINT16), },

			{ "NetworkByteOrderToHostByteOrderL",
			NetworkByteOrderToHostByteOrderObj::Create(NetworkByteOrderToHostByteOrderObj::BO_UINT32), },

			{ "HttpRequest", new ObjectModule<SimpleHttpConnectionObj >, },
		};

		return context->GetLibRegister().RegistLib(
			"cscriptRuntime", slrs, sizeof(slrs) / sizeof(slrs[0])) >= 0;
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* getVersionObj::doCall(doCallContext *context)
	{
		intObject *r = new ObjectModule<intObject>;
		r->mVal = SVN_VERSION;
		return r;
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

	class ReadFileObject : public runtime::baseObjDefault
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
			if (!strcmp(memName, "WriteFile") || !strcmp(memName, "Write"))
			{
				WriteFileObject *o = new runtime::ContainModule<WriteFileObject>(this);
				o->mFileObject = this;
				return o;
			}
			// 原先ReadLine函数废弃了，统一使用Read进行文件的读取
			// 该函数的第一个参数是读取数据的存放string。如果只有一个参数，则函数会读取一行；
			// 如果存在第二个参数，则第二个参数必须是一个正整数，表示读取的字节数
			// 执行后如果返回非空表示执行中没有发生错误，此时，第一个参数指定的string会被返回
			// 可以通过string.len得到读取数据的实际字节长度
			if (!strcmp(memName, "Read"))
			{
				ReadFileObject *o = new ContainModule<ReadFileObject>(this);
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
		size_t toWrite;
		NullTypeObject *nullRet = NullTypeObject::CreateNullTypeObject();
		if (context->GetParamCount() < 1
			|| ((s = context->GetStringParam(0)) == nullptr))
		{
			SCRIPT_TRACE("file.write(strToWrite[, byte count to write]\n");
			return nullRet;
		}
		if (context->GetParamCount() > 1)
		{
			try {
				toWrite = context->GetUint32Param(1);
				if (toWrite > static_cast<stringObject*>(context->GetParam(0))->mVal->size())
				{
					SCRIPT_TRACE("file.Write: write byte count out of range.\n");
					return nullRet;
				}
			}
			catch (...)
			{
				SCRIPT_TRACE("file.Write: second parameter must be an integer\n");
				return nullRet;
			}
		}
		else
			toWrite = strlen(s);
		uintObject *r = new runtime::ObjectModule<uintObject>;
		r->mVal = fwrite(s, 1, toWrite, mFileObject->getFileHandle());
		delete nullRet;
		return r;
	}

	runtime::runtimeObjectBase* ReadFileObject::doCall(doCallContext *context)
	{
		NullTypeObject *nullRet = NullTypeObject::CreateNullTypeObject();
		if (context->GetParamCount() < 1
			|| context->GetParam(0)->GetObjectTypeId() != DT_string)
			return nullRet;
		runtimeObjectBase *o2 = nullptr;
		uint32_t toRead = 0;
		stringObject *s = static_cast<stringObject*>(context->GetParam(0));
		if (context->GetParamCount() > 1)
		{
			o2 = context->GetParam(1);
			if (!isIntegerType(o2))
			{
				SCRIPT_TRACE("file.read: second parameter must be integer larger than 0.\n");
				return nullRet;
			}
			toRead = getObjectDataOrig<int32_t>(o2);
		}
		if (toRead)
		{
			s->mVal->resize(toRead);
			s->mVal->resize(fread(&(*s->mVal)[0], 1, toRead, mFileObject->getFileHandle()));
		}
		else
		{
			if (!notstd::StringHelper::ReadLine<256>(mFileObject->getFileHandle(), *s->mVal, nullptr))
				return nullRet;
		}
		delete nullRet;
		return s;
	}

	runtime::runtimeObjectBase* CloseFileObject::doCall(doCallContext *context)
	{
		runtime::intObject *r = new runtime::ObjectModule<runtime::intObject>;
		r->mVal = fclose(mFileObject->getFileHandle());
		return r;
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* DeleteFileObj::doCall(doCallContext *context)
	{
		NullTypeObject *retNull = NullTypeObject::CreateNullTypeObject();
		if (context->GetParamCount() != 1)
			return retNull;

		const char *filePathName = context->GetStringParam(0);
		if (!filePathName)
			return retNull;

		delete retNull;

		intObject *r = intObject::CreateBaseTypeObject<intObject>(false);
		r->mVal = ::unlink(filePathName);
		return r;
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* CreateBufferObj::doCall(doCallContext *context)
	{
		const char *str;
		uint32_t strLen = 0;
		auto paramCount = context->GetParamCount();

		auto *r = new runtime::ObjectModule<BufferObject>;
		do {
			uint32_t offset = 0, size = -1;

			if (paramCount > 0) {
				str = context->GetStringParam(0, strLen);
				if (!str)
					break;
			}
			if (paramCount > 1) {
				offset = context->GetUint32Param(1);
			}
			if (paramCount > 2) {
				size = context->GetUint32Param(2);
			}

			if (!str)
				return r;

			if (offset >= strLen)
				return r;

			uint32_t actSize = std::min(strLen - offset, size);
			r->mBuffer.append(str + offset, actSize);
			return r;
		} while (0);
		delete r;
		return runtime::NullTypeObject::CreateNullTypeObject();
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* GetTickCountObj::doCall(doCallContext *context)
	{
		return uintObject::CreateUintObject(notstd::GetCurrentTick());
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* ParseInt32Obj::doCall(doCallContext *context)
	{
		const char *str;
		auto paramCount = context->GetParamCount();
		if (paramCount != 1
			|| !(str = context->GetStringParam(0)))
			return intObject::CreateIntObject(0);
		return intObject::CreateIntObject(atoi(str));
	}

	///////////////////////////////////////////////////////////////////////////////

	runtimeObjectBase* ParseUint32Obj::doCall(doCallContext *context)
	{
		const char *str;
		auto paramCount = context->GetParamCount();
		if (paramCount != 1
			|| !(str = context->GetStringParam(0)))
			return uintObject::CreateUintObject(0);
		return uintObject::CreateUintObject(strtoul(str, nullptr, 10));
	}
	
	///////////////////////////////////////////////////////////////////////////////
}
