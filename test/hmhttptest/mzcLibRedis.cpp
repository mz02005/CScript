#include "stdafx.h"
#include "mzcLibRedis.h"
#include <memory>

namespace mzcLib {

	////////////////////////////////////////////////////////////////////////////

	createRedisConnectObj::createRedisConnectObj()
	{
	}

	createRedisConnectObj::~createRedisConnectObj()
	{
	}

	runtime::runtimeObjectBase* createRedisConnectObj::doCall(runtime::doCallContext *context)
	{
		// param1：IP地址
		// param2：端口（缺省为6379）
		// param3：SELECT的数据库，缺省为0
		const char *ip = nullptr;
		int port = 6379;
		int selId = 0;

		do {
			if (context->GetParamCount() < 1)
				break;
			ip = context->GetStringParam(0);
			if (!ip)
				break;
			if (context->GetParamCount() >= 2)
			{
				port = context->GetInt32Param(1);
				if (port >= 65535 || port <= 0)
					break;
			}
			if (context->GetParamCount() >= 3)
			{
				selId = context->GetInt32Param(2);
				if (selId < 0)
					break;
			}

			redisContext *rContext = redisConnect(ip, port);
			if (!rContext)
				break;

			redisReply *reply = (redisReply*)redisCommand(rContext, "SELECT %d", selId);
			if (!reply)
			{
				redisFree(rContext);
				break;
			}

			std::shared_ptr<redisReply> autoFree(reply, freeReplyObject);
			if (reply->type != REDIS_REPLY_STATUS ||
				strcmp(reply->str, "OK") != 0)
			{
				redisFree(rContext);
				break;
			}

			RedisConnectionObj *rc = new runtime::ObjectModule<RedisConnectionObj>;
			rc->mRedisContext = rContext;
			return rc;

		} while (0);
		return runtime::NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////

	RedisDisconnectObj::RedisDisconnectObj()
		: mRedisConn(nullptr)
	{
	}

	RedisDisconnectObj::~RedisDisconnectObj()
	{
		if (mRedisConn)
			mRedisConn->Release();
	}

	runtime::runtimeObjectBase* RedisDisconnectObj::doCall(runtime::doCallContext *context)
	{
		auto r = runtime::intObject::CreateIntObject(-1);
		do {
			if (context->GetParamCount() != 0)
				break;
			if (mRedisConn->mRedisContext)
			{
				redisFree(mRedisConn->mRedisContext);
				mRedisConn->mRedisContext = nullptr;
				r->mVal = 0;
			}
		} while (0);
		return r;
	}

	RedisGetDataObj::RedisGetDataObj()
		: mRedisConn(nullptr)
	{
	}

	RedisGetDataObj::~RedisGetDataObj()
	{
		if (mRedisConn)
			mRedisConn->Release();
	}

	runtime::runtimeObjectBase* RedisGetDataObj::doCall(runtime::doCallContext *context)
	{
		do {
			const char *keyName = nullptr;
			if (context->GetParamCount() != 1
				|| !(keyName = context->GetStringParam(0)))
				break;

			auto result = reinterpret_cast<redisReply*>(redisCommand(mRedisConn->mRedisContext, "GET %s", keyName));
			if (!result)
				break;
			std::shared_ptr<redisReply> autoFree(result, freeReplyObject);

			switch (result->type)
			{
			case REDIS_REPLY_STRING:
				return runtime::stringObject::CreateStringObject(result->str);

				// TODO: 待处理
			case REDIS_REPLY_ARRAY:
				break;

			case REDIS_REPLY_INTEGER:
				return runtime::stringObject::CreateStringObject(std::to_string(result->integer));

			case REDIS_REPLY_NIL:
				break;
				
			case REDIS_REPLY_STATUS:
				break;

			case REDIS_REPLY_ERROR:
				break;
			}
		} while (0);
		return runtime::NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////

	RedisConnectionObj::RedisConnectionObj()
		: mRedisContext(nullptr)
	{
	}

	RedisConnectionObj::~RedisConnectionObj()
	{
		if (mRedisContext)
		{
			::redisFree(mRedisContext);
			mRedisContext = nullptr;
		}
	}

	runtime::runtimeObjectBase* RedisConnectionObj::GetMember(const char *memName)
	{
		if (!strcmp("Disconnect", memName))
		{
			auto r = new runtime::ObjectModule<RedisDisconnectObj>;
			r->mRedisConn = this;
			AddRef();
			return r;
		}
		else if (!strcmp("GetData", memName))
		{
			auto r = new runtime::ObjectModule<RedisGetDataObj>;
			r->mRedisConn = this;
			AddRef();
			return r;
		}
		return runtime::baseObjDefault::GetMember(memName);
	}

	////////////////////////////////////////////////////////////////////////////
}
