#pragma once
#include "config.h"
#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif
#include "mysql.h"
#include <inttypes.h>
#include <string>
#include <vector>
#include <functional>
#include <ostream>
#include "notstd/notstd.h"

class MysqlResult;
class MysqlConnection;
class MysqlStatementPrepare;
class MysqlResultForPrepare;

typedef std::function<void(const char *errInfo)> MysqlOnErrorProc;

MYSQL_HELPER_API notstd::CStringA MySqlTime2String(const MYSQL_TIME *mt);

template class MYSQL_HELPER_API notstd::Array<MYSQL_BIND>;

class MYSQL_HELPER_API MysqlResultForPrepare
{
protected:
	MysqlConnection *mConn;
	MYSQL_STMT *mMysqlStmt;
	//MYSQL_RES *mMysqlRes;
	notstd::CStringA mStatement;
	notstd::Array<MYSQL_BIND> mMysqlBinds;

public:
	typedef unsigned long ColumnLength;

	MysqlResultForPrepare(MysqlConnection *mysqlConn, const char *statement);
	~MysqlResultForPrepare();

	void Destroy();

	int PrepareForResult(MysqlOnErrorProc onErr = nullptr);
	int Select(bool cacheDataOnClient = false, MysqlOnErrorProc onErr = nullptr);

	void AddBind(uint64_t *val, ColumnLength *l);
	void AddBind(int32_t *val, ColumnLength *l);
	void AddString(char *val, unsigned long len, ColumnLength *l);
	void AddBlob(char *val, unsigned long len, ColumnLength *l);
	void AddDatetime(MYSQL_TIME *val, ColumnLength *l);
	int MoveFirst();
	int MoveNext(MysqlOnErrorProc onErr = nullptr);
};

class MYSQL_HELPER_API MysqlStatementPrepare
{
protected:
	MysqlConnection *mConn;
	MYSQL_STMT *mMysqlStmt;
	notstd::CStringA mStatement;
	notstd::Array<MYSQL_BIND> mMysqlBinds;

public:
	MysqlStatementPrepare(MysqlConnection *mysqlConn, const char *statement);
	~MysqlStatementPrepare();

	void Destroy();

	// 这组函数用于执行不返回数据的
	void AddBind(uint64_t *val);
	void AddBind(int32_t *val);
	void AddString(char *val, size_t size);
	void AddBlob(char *val, size_t size);
	void AddDatetime(MYSQL_TIME *val);
	int Prepare(MysqlOnErrorProc f = nullptr);
	int Execute(MysqlOnErrorProc f = nullptr);
};

class MYSQL_HELPER_API MysqlResult
{
	friend class MysqlConnection;

private:
	MysqlConnection *mConn;
	MYSQL_RES *mRes;
	uint32_t mFieldNum;
	uint64_t mRowNum;

	MYSQL_ROW mRow;
	unsigned long *mLengths;

public:
	MysqlResult();
	~MysqlResult();

	void Reset();
	
	int MoveToFirst(MysqlOnErrorProc onError = nullptr);
	uint64_t GetRowCount() const;
	uint32_t GetColumnCount() const;

	int MoveNext(MysqlOnErrorProc onError = nullptr);
	int Seek(uint64_t off);

	int32_t GetInt32Data(uint32_t field, int *isError = nullptr);
	int64_t GetInt64Data(uint32_t field, int *isError = nullptr);
	uint32_t GetUint32Data(uint32_t field, int *isError = nullptr);
	uint64_t GetUint64Data(uint32_t field, int *isError = nullptr);
	std::string GetString(uint32_t field, int *isError = nullptr);
	MYSQL_TIME GetTime(uint32_t field, int *isError = nullptr);
};

class MYSQL_HELPER_API MysqlConnection
{
	MYSQL *mMysql;

public:
	struct MYSQL_HELPER_API MysqlConf
	{
		// 只能是IP地址
		char host[16];

		char username[32];
		char password[32];

		char dbName[64];

		uint16_t port;

		uint16_t reserved : 15;

		// 不要自动重连
		uint16_t doNotAutoConnect : 1;

		MysqlConf();
		MysqlConf(const char *n, const char *p, const char *db, 
			const char *host = "127.0.0.1", uint16_t P = 0);
	};

public:
	MysqlConnection();
	~MysqlConnection();

	operator MYSQL*() { return mMysql; }

	bool isConnected();
	int Connect(const MysqlConf &conf);
	void Shutdown();

	int Select(const char *sqlstatement, MysqlResult *result, MysqlOnErrorProc onError = nullptr);

	// 返回>=0表示成功及影响的行数；否则返回<0的值表示错误
	int Execute(const char *sqlstatement, MysqlOnErrorProc onError = nullptr);

	int BeginTrans();
	int CommitTrans();
	int RollbackTrans();
};
