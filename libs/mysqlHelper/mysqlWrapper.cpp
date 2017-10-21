#include "stdafx.h"
#include "mysqlWrapper.h"
#include <climits>
#include <limits.h>
#include <iomanip>

class MysqlFieldData
{
private:
	char mDefaultCache[512];
	uint32_t mSize;
	char *mData;

public:
	MysqlFieldData(const char *buf, uint32_t size)
		: mData(mDefaultCache)
		, mSize(size)
	{
		if (size >= sizeof(mDefaultCache))
			mData = (char*)malloc(size + 1);
		if (size)
			memcpy(mData, buf, size);
		mData[size] = 0;
	}

	~MysqlFieldData()
	{
		if (mData != mDefaultCache)
			free(mData);
	}

	operator const char*()
	{
		return mData;
	}

	uint32_t GetSize() const {
		return mSize;
	}
};

////////////////////////////////////////////////////////////////////////////////

MYSQL_HELPER_API notstd::CStringA MySqlTime2String(const MYSQL_TIME *mt)
{
	char buf[20];
	sprintf_s(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d",
		static_cast<int>(mt->year), static_cast<int>(mt->month), static_cast<int>(mt->day),
		static_cast<int>(mt->hour), static_cast<int>(mt->minute), static_cast<int>(mt->second));
	return buf;
}

MYSQL_HELPER_API std::ostream& operator << (std::ostream &os, const MYSQL_TIME &mt)
{
	os << std::setw(2) << std::setfill('0') << mt.year << '-' << mt.month << '-'
		<< mt.day << ' ' << mt.hour << ':' << mt.minute << ':' << mt.second;
	return os;
}

////////////////////////////////////////////////////////////////////////////////

MysqlStatementPrepare::MysqlStatementPrepare(MysqlConnection *mysqlConn, const char *statement)
	: mConn(mysqlConn)
	, mStatement(statement)
{
	mMysqlStmt = mysql_stmt_init(*mysqlConn);
}

MysqlStatementPrepare::~MysqlStatementPrepare()
{
	Destroy();
}

void MysqlStatementPrepare::AddBind(uint64_t *val)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_LONGLONG;
	mb.is_unsigned = 1;
	mb.buffer = val;
	mMysqlBinds.Add(mb);
}

void MysqlStatementPrepare::AddBind(int32_t *val)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_LONG;
	mb.buffer = val;
	mb.is_null = 0;
	mMysqlBinds.Add(mb);
}

void MysqlStatementPrepare::AddString(char *val, size_t size)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_STRING;
	mb.buffer = val;
	mb.buffer_length = size;
	mMysqlBinds.Add(mb);
}

void MysqlStatementPrepare::AddBlob(char *val, size_t size)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_BLOB;
	mb.buffer = val;
	mb.buffer_length = size;
	mMysqlBinds.Add(mb);
}

void MysqlStatementPrepare::AddDatetime(MYSQL_TIME *val)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_DATETIME;
	mb.buffer = val;
	mMysqlBinds.Add(mb);
}

int MysqlStatementPrepare::Prepare(MysqlOnErrorProc f)
{
	if (!mMysqlBinds.GetSize())
		return -2;

	int r = 0;
	do {
		if (mysql_stmt_prepare(mMysqlStmt, mStatement.c_str(), mStatement.GetLength()))
		{
			r = -3;
			break;
		}
		if (mysql_stmt_bind_param(mMysqlStmt, &mMysqlBinds[0]))
		{
			r = -4;
			break;
		}
		if (mysql_stmt_param_count(mMysqlStmt) != mMysqlBinds.GetSize())
		{
			r = -5;
			break;
		}
		return r;
	} while (0);
	if (f)
		f(mysql_stmt_error(mMysqlStmt));
	return r;
}

int MysqlStatementPrepare::Execute(MysqlOnErrorProc f)
{
	auto r = mysql_stmt_execute(mMysqlStmt);
	if (r)
	{
		if (f)
			f(mysql_error(*mConn));
		return -1;
	}
	auto r2 = mysql_stmt_affected_rows(mMysqlStmt);
	return r2 > INT_MAX ? -2 : static_cast<int>(r2);
}

void MysqlStatementPrepare::Destroy()
{
	if (mMysqlStmt)
	{
		mysql_stmt_close(mMysqlStmt);
		mMysqlStmt = nullptr;
	}
	mMysqlBinds.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////////

MysqlResultForPrepare::MysqlResultForPrepare(MysqlConnection *mysqlConn, const char *statement)
	: mConn(mysqlConn)
	, mStatement(statement)
	//, mMysqlRes(nullptr)
{
	mMysqlStmt = mysql_stmt_init(*mysqlConn);
}

MysqlResultForPrepare::~MysqlResultForPrepare()
{
	if (mMysqlStmt)
	{
		mysql_stmt_close(mMysqlStmt);
		mMysqlStmt = nullptr;
	}
	Destroy();
}

void MysqlResultForPrepare::Destroy()
{
	//if (mMysqlRes)
	//{
	//	mysql_free_result(mMysqlRes);
	//	mMysqlRes = nullptr;
	//}
	mMysqlBinds.RemoveAll();
}

int MysqlResultForPrepare::PrepareForResult(MysqlOnErrorProc onErr)
{
	if (mysql_stmt_prepare(mMysqlStmt, mStatement.c_str(), mStatement.GetLength()))
	{
		if (onErr)
			onErr(mysql_stmt_error(mMysqlStmt));
		return -1;
	}
	return 0;
}

int MysqlResultForPrepare::Select(bool cacheDataOnClient, MysqlOnErrorProc onErr)
{
	int r = 0;
	do {
		//mMysqlRes = mysql_stmt_result_metadata(mMysqlStmt);
		if (!mMysqlStmt)
		{
			r = -1;
			break;
		}
		if (mysql_stmt_execute(mMysqlStmt))
		{
			r = -2;
			break;
		}
		if (mysql_stmt_bind_result(mMysqlStmt, &mMysqlBinds[0]))
		{
			r = -3;
			break;
		}
		if (cacheDataOnClient)
		{
			if (mysql_stmt_store_result(mMysqlStmt))
			{
				r = -4;
				break;
			}
		}
		return MoveFirst();
	} while (0);
	if (onErr)
		onErr(mysql_stmt_error(mMysqlStmt));
	return r;
}

void MysqlResultForPrepare::AddBind(uint64_t *val, ColumnLength *l)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_LONGLONG;
	mb.is_unsigned = 1;
	mb.buffer = val;
	mb.length = l;
	mb.buffer_length = sizeof(uint64_t);
	mMysqlBinds.Add(mb);
}

void MysqlResultForPrepare::AddBind(int32_t *val, ColumnLength *l)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_LONG;
	mb.buffer = val;
	mb.is_null = 0;
	mb.length = l;
	mb.buffer_length = sizeof(int32_t);
	mMysqlBinds.Add(mb);
}

void MysqlResultForPrepare::AddString(char *val, unsigned long len, ColumnLength *l)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_STRING;
	mb.buffer = val;
	mb.length = l;
	mb.buffer_length = len;
	mMysqlBinds.Add(mb);
}

void MysqlResultForPrepare::AddBlob(char *val, unsigned long len, ColumnLength *l)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_BLOB;
	mb.buffer = val;
	mb.length = l;
	mb.buffer_length = len;
	mMysqlBinds.Add(mb);
}

void MysqlResultForPrepare::AddDatetime(MYSQL_TIME *val, ColumnLength *l)
{
	MYSQL_BIND mb;
	memset(&mb, 0, sizeof(mb));
	mb.buffer_type = MYSQL_TYPE_DATETIME;
	mb.buffer = val;
	mb.length = l;
	mb.buffer_length = sizeof(int32_t);
	mMysqlBinds.Add(mb);
}

int MysqlResultForPrepare::MoveFirst()
{
	// Do nothing
	return 0;
}

int MysqlResultForPrepare::MoveNext(MysqlOnErrorProc onErr)
{
	int r = mysql_stmt_fetch(mMysqlStmt);
	if (r && onErr)
		onErr(mysql_stmt_error(mMysqlStmt));
	return r ? -1 : 0;
}

////////////////////////////////////////////////////////////////////////////////

MysqlResult::MysqlResult()
	: mConn(nullptr)
	, mRes(nullptr)
	, mFieldNum(0)
	, mRowNum(0)
	, mRow(0)
	, mLengths(nullptr)
{
}

MysqlResult::~MysqlResult()
{
	Reset();
}

void MysqlResult::Reset()
{
	if (mRes) {
		mysql_free_result(mRes);
		mRes = nullptr;
	}
	mFieldNum = 0;
	mRowNum = 0;
	mRow = 0;
	mLengths = nullptr;
}

int MysqlResult::Seek(uint64_t off)
{
	mysql_data_seek(mRes, off);
	return 0;
}

int MysqlResult::MoveToFirst(MysqlOnErrorProc onError)
{
	if (!mRes)
	{
		return -1;
	}

	mFieldNum = mysql_num_fields(mRes);
	if (!mFieldNum)
	{
		return -2;
	}

	mRowNum = mysql_num_rows(mRes);
	if (!mRowNum)
	{
		return -3;
	}

	return Seek(0);
}

uint64_t MysqlResult::GetRowCount() const {
	return mRowNum;
}

uint32_t MysqlResult::GetColumnCount() const {
	return mFieldNum;
}

int MysqlResult::MoveNext(MysqlOnErrorProc onError)
{
	mRow = mysql_fetch_row(mRes);
	if (!mRow) {
		if (onError)
			onError(mysql_error(*mConn));
		return -1;
	}
	mLengths = mysql_fetch_lengths(mRes);
	return 0;
}

#define SET_1_IF_NOTNULL(e,v) do { \
		if ((e)) { *(e) = 1; return (v); } \
				} while (0)

#define SET_0_IF_NOT_NULL(e) do { if ((e)) *(e) = 0; } while (0)

int32_t MysqlResult::GetInt32Data(uint32_t field, int *isError)
{
	if (field >= mFieldNum
		|| !mRow)
		SET_1_IF_NOTNULL(isError, 0);

	MysqlFieldData data(mRow[field], mLengths[field]);
	char *e = nullptr;
	int32_t r = strtol(data, &e, 10);
	if (e - data != data.GetSize())
		SET_1_IF_NOTNULL(isError, 0);

	SET_0_IF_NOT_NULL(isError);
	return r;
}

int64_t MysqlResult::GetInt64Data(uint32_t field, int *isError)
{
	if (field >= mFieldNum
		|| !mRow)
		SET_1_IF_NOTNULL(isError, 0);

	MysqlFieldData data(mRow[field], mLengths[field]);
	char *e = nullptr;
	int64_t r = strtoll(data, &e, 10);
	if (e - data != data.GetSize())
		SET_1_IF_NOTNULL(isError, 0);

	SET_0_IF_NOT_NULL(isError);
	return r;
}

uint32_t MysqlResult::GetUint32Data(uint32_t field, int *isError)
{
	if (field >= mFieldNum
		|| !mRow)
		SET_1_IF_NOTNULL(isError, 0);

	MysqlFieldData data(mRow[field], mLengths[field]);
	char *e = nullptr;
	uint32_t r = strtoul(data, &e, 10);
	if (e - data != data.GetSize())
		SET_1_IF_NOTNULL(isError, 0);

	SET_0_IF_NOT_NULL(isError);
	return r;
}

uint64_t MysqlResult::GetUint64Data(uint32_t field, int *isError)
{
	if (field >= mFieldNum
		|| !mRow)
		SET_1_IF_NOTNULL(isError, 0);

	MysqlFieldData data(mRow[field], mLengths[field]);
	char *e = nullptr;
	uint64_t r = strtoull(data, &e, 10);
	if (e - data != data.GetSize())
		SET_1_IF_NOTNULL(isError, 0);

	SET_0_IF_NOT_NULL(isError);
	return r;
}

MYSQL_TIME MysqlResult::GetTime(uint32_t field, int *isError)
{
	MYSQL_TIME r;
	memset(&r, 0, sizeof(r));

	if (field >= mFieldNum
		|| !mRow)
		SET_1_IF_NOTNULL(isError, r);

	static const char simpleDate[] = "0000-00-00 00:00:00";
	if (mLengths[field] < sizeof(simpleDate) - 1)
		SET_1_IF_NOTNULL(isError, r);

	MysqlFieldData data(mRow[field], mLengths[field]);
	sscanf(data.operator const char *(), "%04d-%04d-%04d %02d:%02d:%02d",
		&r.year, &r.month, &r.day, &r.hour, &r.minute, &r.second);

	return r;
}

std::string MysqlResult::GetString(uint32_t field, int *isError)
{
	if (field >= mFieldNum
		|| !mRow)
		SET_1_IF_NOTNULL(isError, 0);

	SET_0_IF_NOT_NULL(isError);
	return mRow[field] ? std::string(mRow[field], mRow[field] + mLengths[field]) : "";
}

////////////////////////////////////////////////////////////////////////////////

MysqlConnection::MysqlConf::MysqlConf()
{
	memset(this, 0, sizeof(*this));
}

MysqlConnection::MysqlConf::MysqlConf(
	const char *n, const char *p, const char *db,
	const char *h, uint16_t P)
{
	memset(this, 0, sizeof(*this));
	strncpy_s(host,		sizeof(host),		h,	_TRUNCATE);
	strncpy_s(username, sizeof(username),	n,	_TRUNCATE);
	strncpy_s(password, sizeof(password),	p,	_TRUNCATE);
	strncpy_s(dbName,	sizeof(dbName),		db,	_TRUNCATE);
	port = P;
}

MysqlConnection::MysqlConnection()
	: mMysql(nullptr)
{
}

MysqlConnection::~MysqlConnection()
{
	Shutdown();
}

void MysqlConnection::Shutdown()
{
	if (mMysql) {
		mysql_close(mMysql);
		mMysql = nullptr;
	}
}

int MysqlConnection::Connect(const MysqlConf &conf)
{
	Shutdown();

	mMysql = mysql_init(nullptr);
	if (!mMysql)
		return -1;

	int ret;
	do {
		if (!mysql_real_connect(mMysql, conf.host, conf.username,
			conf.password, conf.dbName, conf.port ? conf.port : 3306, 
			nullptr, CLIENT_MULTI_STATEMENTS))
		{
			ret = -2;
			break;
		}

		char value = 1;
		if (!conf.doNotAutoConnect)
			mysql_options(mMysql, MYSQL_OPT_RECONNECT, &value);

		mysql_set_character_set(mMysql, "gbk");
		return 0;
	} while (0);

	Shutdown();

	return ret;
}

bool MysqlConnection::isConnected()
{
	if (!mMysql)
		return false;
	return mysql_ping(mMysql) == 0;
}

int MysqlConnection::Select(const char *sqlstatement, MysqlResult *result, MysqlOnErrorProc onError)
{
	MYSQL_RES *res;
	if (mysql_query(mMysql, sqlstatement) != 0)
	{
		if (onError)
			onError(mysql_error(mMysql));
		return -1;
	}

	if ((res = mysql_store_result(mMysql)) == nullptr)
	{
		if (onError)
			onError(mysql_error(mMysql));
		return -2;
	}

	result->mRes = res;
	return result->MoveToFirst();
}

int MysqlConnection::Execute(const char *sqlstatement, MysqlOnErrorProc onError)
{
	if (mysql_query(mMysql, sqlstatement) != 0) {
		if (onError)
			onError(mysql_error(mMysql));
		return -1;
	}
	uint64_t affect = mysql_affected_rows(mMysql);
	int r = static_cast<int>(affect);
	r = static_cast<uint64_t>(r) == affect ? r : -2;

	if (r < 0 && onError)
		onError(mysql_error(mMysql));

	return r;
}

int MysqlConnection::BeginTrans()
{
	return mysql_autocommit(mMysql, 0) ? 0 : -1;
}

int MysqlConnection::CommitTrans()
{
	auto r = mysql_commit(mMysql);
	if (r)
		mysql_autocommit(mMysql, 1);
	return r ? 0 : -1;
}

int MysqlConnection::RollbackTrans()
{
	auto r = mysql_rollback(mMysql);
	if (r)
		mysql_autocommit(mMysql, 1);
	return r ? 0 : -1;
}
