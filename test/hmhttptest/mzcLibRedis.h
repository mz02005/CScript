#pragma once
//#include "hiredis.h"
#include "CScriptEng/vm.h"
#include "CScriptEng/arrayType.h"
#include "hiredis.h"

namespace mzcLib {

	class RedisConnectionObj;

	class RedisDisconnectObj : public runtime::baseObjDefault
	{
		friend class RedisConnectionObj;

	private:
		RedisConnectionObj *mRedisConn;

	public:
		RedisDisconnectObj();
		virtual ~RedisDisconnectObj();
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class RedisGetDataObj : public runtime::baseObjDefault
	{
		friend class RedisConnectionObj;

	private:
		RedisConnectionObj *mRedisConn;

	public:
		RedisGetDataObj();
		virtual ~RedisGetDataObj();
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class RedisConnectionObj : public runtime::baseObjDefault
	{
		friend class createRedisConnectObj;
		friend class RedisDisconnectObj;
		friend class RedisGetDataObj;

	private:
		redisContext *mRedisContext;

	public:
		RedisConnectionObj();
		virtual ~RedisConnectionObj();
		virtual runtime::runtimeObjectBase* GetMember(const char *memName) override;
	};

	class createRedisConnectObj : public runtime::baseObjDefault
	{
	public:
		createRedisConnectObj();
		virtual ~createRedisConnectObj();
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};
}
