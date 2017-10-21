#include "stdafx.h"
#include "mzcLibMysql.h"
#include "mysqlWrapper.h"

namespace mzcLib {
	class MysqlConnectionObj;
	class createMysqlConnectionObj;
	class destroyMysqlConnectionObj;
	class ConnectionSelectObj;
	class MysqlResultObj;
	class GetTimeDataObj;

	class ResultMoveFirstObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		ResultMoveFirstObj()
			: mMysqlResult(nullptr)
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::intObject>;
			r->mVal = mMysqlResult->MoveToFirst();
			return r;
		}
	};

	class ResultMoveNextObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		ResultMoveNextObj()
			: mMysqlResult(nullptr)
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::intObject>;
			r->mVal = mMysqlResult->MoveNext();
			return r;
		}
	};

	class MYSQLTimeObj : public runtime::baseTypeObject
	{
		friend class GetTimeDataObj;

	private:
		MYSQL_TIME mVal;

	public:
		MYSQLTimeObj()
		{
			memset(&mVal, 0, sizeof(mVal));
		}

		runtime::runtimeObjectBase* GetMember(const char *memName)
		{
			// 得到32位unix时间格式（整数）
			if (!strcmp("UnixTime", memName))
			{
				tm theTm = { 0 };
				theTm.tm_year = mVal.year - 1900;
				theTm.tm_mon = mVal.month - 1;
				theTm.tm_mday = mVal.day;
				theTm.tm_hour = mVal.hour;
				theTm.tm_min = mVal.minute;
				theTm.tm_sec = mVal.second;
				return runtime::intObject::CreateIntObject(static_cast<int>(mktime(&theTm)));
			}
			return runtime::baseTypeObject::GetMember(memName);
		}

		virtual runtime::stringObject* toString()
		{
			auto *r = new runtime::ObjectModule<runtime::stringObject>;
			*r->mVal = MySqlTime2String(&mVal).c_str();
			return r;
		}
	};

	class GetTimeDataObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		GetTimeDataObj()
			: mMysqlResult(nullptr)
		{
		}
		
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::arrayObject>;
			if (context->GetParamCount() < 1)
			{
				r->AddSub(runtime::intObject::CreateIntObject(-1));
				return r;
			}
			int isError = 0;
			MYSQL_TIME mt = mMysqlResult->GetTime(context->GetInt32Param(0), &isError);
			r->AddSub(runtime::intObject::CreateIntObject(isError ? -1 : 0));
			auto *theVal = new runtime::ObjectModule<MYSQLTimeObj>;
			theVal->mVal = mt;
			r->AddSub(theVal);
			return r;
		}
	};

	class GetStringDataObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		GetStringDataObj()
			: mMysqlResult(nullptr)
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::arrayObject>;
			if (context->GetParamCount() < 1)
			{
				r->AddSub(runtime::intObject::CreateIntObject(-1));
				return r;
			}
			int isError = 0;
			std::string result = mMysqlResult->GetString(context->GetInt32Param(0));
			r->AddSub(runtime::intObject::CreateIntObject(isError ? -1 : 0));
			auto *theVal = new runtime::ObjectModule<runtime::stringObject>;
			*theVal->mVal = result;
			r->AddSub(theVal);
			return r;
		}
	};

	class GetUint32DataObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		GetUint32DataObj()
			: mMysqlResult(nullptr)
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::arrayObject>;

			if (context->GetParamCount() < 1)
			{
				r->AddSub(runtime::intObject::CreateIntObject(-1));
				return r;
			}
			
			int isError = 0;
			uint32_t result = mMysqlResult->GetUint32Data(context->GetInt32Param(0));
			r->AddSub(runtime::intObject::CreateIntObject(isError ? -1 : 0));
			auto *theVal = new runtime::ObjectModule<runtime::uintObject>;
			theVal->mVal = result;
			r->AddSub(theVal);
			return r;
		}
	};

	class GetUint64DataObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		GetUint64DataObj()
			: mMysqlResult(nullptr)
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::arrayObject>;
			if (context->GetParamCount() < 1)
			{
				r->AddSub(runtime::intObject::CreateIntObject(-1));
				return r;
			}
			int isError = 0;
			uint64_t result = mMysqlResult->GetUint64Data(context->GetInt32Param(0));
			// 第一个表示是否成功，>=0表示成功
			r->AddSub(runtime::intObject::CreateIntObject(isError ? -1 : 0));
			// 第二个表示低32位，32位无符号整数
			auto *lowPart = new runtime::ObjectModule<runtime::uintObject>;
			lowPart->mVal = static_cast<uint32_t>(result & 0x00000000FFFFFFFF);
			r->AddSub(lowPart);
			// 第三个表示高32位，32位无符号整数
			auto *highPart = new runtime::ObjectModule<runtime::uintObject>;
			highPart->mVal = static_cast<uint32_t>((result & 0xFFFFFFFF00000000) >> 32);
			r->AddSub(highPart);
			return r;
		}
	};

	class GetInt64DataObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		GetInt64DataObj()
			: mMysqlResult(nullptr)
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::arrayObject>;
			if (context->GetParamCount() < 1)
			{
				r->AddSub(runtime::intObject::CreateIntObject(-1));
				return r;
			}
			int isError = 0;
			int64_t result = mMysqlResult->GetInt32Data(context->GetInt32Param(0));
			// 第一个表示是否成功，>=0表示成功
			r->AddSub(runtime::intObject::CreateIntObject(isError ? -1 : 0));
			// 第二个表示低32位，32位无符号整数
			auto *lowPart = new runtime::ObjectModule<runtime::uintObject>;
			lowPart->mVal = static_cast<uint32_t>(result & 0x00000000FFFFFFFF);
			r->AddSub(lowPart);
			// 第三个表示高32位，32位带符号整数
			r->AddSub(runtime::intObject::CreateIntObject(
				static_cast<int32_t>((result & 0xFFFFFFFF00000000) >> 32)
				));
			return r;
		}
	};

	class GetInt32DataObj : public runtime::baseTypeObject
	{
		friend class MysqlResultObj;

	private:
		MysqlResult *mMysqlResult;

	public:
		GetInt32DataObj()
			: mMysqlResult(nullptr)
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			auto *r = new runtime::ObjectModule<runtime::arrayObject>;
			if (context->GetParamCount() < 1)
			{
				r->AddSub(runtime::intObject::CreateIntObject(-1));
				return r;
			}
			int isError = 0;
			int32_t result = mMysqlResult->GetInt32Data(context->GetInt32Param(0));
			r->AddSub(runtime::intObject::CreateIntObject(isError ? -1 : 0));
			r->AddSub(runtime::intObject::CreateIntObject(result));
			return r;
		}
	};

	class ConnectionExecuteObj : public runtime::baseTypeObject
	{
		friend class MysqlConnectionObj;

	private:
		MysqlConnection *mMysqlConn;

	public:
		ConnectionExecuteObj()
			: mMysqlConn(nullptr)
		{
		}

		virtual ~ConnectionExecuteObj()
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			// 唯一的参数是select语句
			if (context->GetParamCount() < 1)
				return runtime::NullTypeObject::CreateNullTypeObject();

			const char *sqlstr = context->GetStringParam(0);
			if (!sqlstr)
				return runtime::NullTypeObject::CreateNullTypeObject();

			// 返回受影响的行数表示成功；否则返回负数表示失败
			auto * r = runtime::intObject::CreateIntObject(mMysqlConn->Execute(sqlstr));
			return r;
		}
	};

	class MysqlResultObj : public runtime::baseTypeObject
	{
		friend class ConnectionSelectObj;

	private:
		ConnectionSelectObj *mConnObj;
		MysqlResult *mMysqlResult;
		int mResult;
		std::string mErrString;

	public:
		MysqlResultObj();
		virtual ~MysqlResultObj();

		virtual runtime::runtimeObjectBase* GetMember(const char *memName);
	};

	class ConnectionSelectObj : public runtime::baseTypeObject
	{
		friend class MysqlConnectionObj;

	private:
		MysqlConnection *mMysqlConn;

	public:
		ConnectionSelectObj()
			: mMysqlConn(nullptr)
		{
		}

		virtual ~ConnectionSelectObj()
		{
		}

		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			// 唯一的参数是select语句
			if (context->GetParamCount() < 1)
				return runtime::NullTypeObject::CreateNullTypeObject();

			const char *sqlstr = context->GetStringParam(0);
			if (!sqlstr)
				return runtime::NullTypeObject::CreateNullTypeObject();

			MysqlResult *selResult = new MysqlResult;
			std::string errCode;
			int result = mMysqlConn->Select(sqlstr, selResult, [&](const char *err)
			{
				errCode = err;
			});

			auto *r = new runtime::ObjectModule<MysqlResultObj>;
			r->mConnObj = this;
			AddRef();
			r->mMysqlResult = selResult;
			r->mResult = result;
			r->mMysqlResult = selResult;
			r->mErrString = errCode;

			return r;
		}
	};

	class MysqlConnectionObj : public runtime::baseTypeObject
	{
		friend class createMysqlConnectionObj;
		friend class destroyMysqlConnectionObj;

	private:
		MysqlConnection *mMysqlConn;

	public:
		MysqlConnectionObj()
			: mMysqlConn(nullptr)
		{
		}

		~MysqlConnectionObj()
		{
			if (mMysqlConn)
			{
				mMysqlConn->Shutdown();
				delete mMysqlConn;
			}
		}

		virtual runtime::runtimeObjectBase* GetMember(const char *memName)
		{
			if (!strcmp("Select", memName))
			{
				// 静态成员，不需要保存父对象
				auto *r = new runtime::ObjectModule<ConnectionSelectObj>;
				r->mMysqlConn = mMysqlConn;
				return r;
			}
			else if (!strcmp("Execute", memName))
			{
				// 静态成员，不需要保存父对象
				auto *r = new runtime::ObjectModule<ConnectionExecuteObj>;
				r->mMysqlConn = mMysqlConn;
				return r;
			}
			return runtime::baseTypeObject::GetMember(memName);
		}
	};
	
	class createMysqlConnectionObj : public runtime::baseTypeObject
	{
	public:
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			// username,password,dbname,hostname,port
			// 其中，前面三个参数必须有
			const char *username, *password, *dbName, *hostName;
			uint16_t port = 3306;
			auto paramCount = context->GetParamCount();
			if (paramCount < 3
				|| (username = context->GetStringParam(0)) == nullptr
				|| (password = context->GetStringParam(1)) == nullptr
				|| (dbName = context->GetStringParam(2)) == nullptr)
				return runtime::NullTypeObject::CreateNullTypeObject();

			if (paramCount > 3)
				hostName = context->GetStringParam(3);
			if (paramCount > 4)
				port = context->GetUint16Param(4);

			MysqlConnection *conn = new MysqlConnection;
			if (conn->Connect(MysqlConnection::MysqlConf(username, password, dbName, hostName, port)) < 0)
			{
				delete conn;
				return runtime::NullTypeObject::CreateNullTypeObject();
			}
			auto *r = new runtime::ObjectModule<MysqlConnectionObj>;
			r->mMysqlConn = conn;
			
			return r;
		}
	};
	
	runtime::runtimeObjectBase* mysqlObject::GetMember(const char *memName)
	{
		if (!strcmp("createMysqlConnection", memName))
		{
			// 静态成员，不需要保存父对象
			return new runtime::ObjectModule<createMysqlConnectionObj>;
		}
		return runtime::baseObjDefault::GetMember(memName);
	}

	////////////////////////////////////////////////////////////////////////////

	MysqlResultObj::MysqlResultObj()
		: mMysqlResult(nullptr)
		, mConnObj(nullptr)
		, mResult(-1)
	{
	}

	MysqlResultObj::~MysqlResultObj()
	{
		if (mMysqlResult)
		{
			delete mMysqlResult;
		}
		if (mConnObj)
		{
			mConnObj->Release();
		}
	}

	runtime::runtimeObjectBase* MysqlResultObj::GetMember(const char *memName)
	{
		if (!strcmp("Result", memName))
		{
			auto *r = new runtime::ObjectModule<runtime::intObject>;
			r->mVal = mResult;
			return r;
		}
		else if (!strcmp("ErrorMessage", memName))
		{
			auto *r = new runtime::ObjectModule<runtime::stringObject>;
			*r->mVal = mErrString;
			return r;
		}
		else if (!strcmp("RowCount", memName))
		{
			auto *r = new runtime::ObjectModule<runtime::uintObject>;
			r->mVal = static_cast<uint32_t>(mMysqlResult->GetRowCount());
			return r;
		}
		else if (!strcmp("MoveFirst", memName))
		{
			auto *r = new runtime::ObjectModule<ResultMoveFirstObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		else if (!strcmp("MoveNext", memName))
		{
			auto *r = new runtime::ObjectModule<ResultMoveNextObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		else if (!strcmp("GetInt32Data", memName))
		{
			auto *r = new runtime::ObjectModule<GetInt32DataObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		else if (!strcmp("GetUint64Data", memName))
		{
			auto *r = new runtime::ObjectModule<GetUint64DataObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		else if (!strcmp("GetUint32Data", memName))
		{
			auto *r = new runtime::ObjectModule<GetUint32DataObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		else if (!strcmp("GetInt64Data", memName))
		{
			auto *r = new runtime::ObjectModule<GetInt64DataObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		else if (!strcmp("GetStringData", memName))
		{
			auto *r = new runtime::ObjectModule<GetStringDataObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		else if (!strcmp("GetTimeData", memName))
		{
			auto *r = new runtime::ObjectModule<GetTimeDataObj>;
			r->mMysqlResult = mMysqlResult;
			return r;
		}
		return runtime::baseTypeObject::GetMember(memName);
	}

}
