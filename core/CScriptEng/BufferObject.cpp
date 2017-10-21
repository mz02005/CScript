#include "stdafx.h"
#include "BufferObject.h"
#include "objType.h"
#include <algorithm>
#include <string.h>

#ifdef min
#undef min
#endif

namespace runtime {
	////////////////////////////////////////////////////////////////////////////

	BufferTruncateObj::BufferTruncateObj()
		: mBufferObj(nullptr)
	{
	}

	BufferTruncateObj::~BufferTruncateObj()
	{
		if (mBufferObj)
			mBufferObj->Release();
	}

	runtime::runtimeObjectBase* BufferTruncateObj::doCall(doCallContext *context)
	{
		do {
			if (context->GetParamCount() != 1)
				break;
			auto newLen = context->GetUint32Param(0);
			mBufferObj->mBuffer.resize(newLen);
			if (mBufferObj->mPosition > newLen)
				mBufferObj->mPosition = newLen;
			return mBufferObj;
		} while (0);
		return NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////

	BufferReadLineObj::BufferReadLineObj()
		: mBufferObj(nullptr)
	{
	}

	BufferReadLineObj::~BufferReadLineObj()
	{
		if (mBufferObj)
			mBufferObj->Release();
	}

	runtime::runtimeObjectBase* BufferReadLineObj::doCall(doCallContext *context)
	{
		do {
			if (context->GetParamCount() != 0)
				break;
			auto *r = new ObjectModule<stringObject>;
			const char *b = mBufferObj->mBuffer.c_str() + mBufferObj->mPosition;
			const char *p = b;
			if (*p == 0)
				break;
			for (; *p != 0; p++)
			{
				if (*p == '\r')
				{
					if (*(p + 1) == '\n')
					{
						r->mVal->append(b, p - b);
						mBufferObj->mPosition += p - b + 2;
						break;
					}
				}
				else if (*p == '\n')
				{
					r->mVal->append(b, p - b);
					mBufferObj->mPosition += p - b + 1;
					break;
				}
			}
			if (*p == 0)
			{
				r->mVal->append(b, p - b);
				mBufferObj->mPosition += p - b;
			}
			return r;
		} while (0);
		return NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////

	BufferSeekToObj::BufferSeekToObj()
		: mBufferObj(nullptr)
	{
	}

	BufferSeekToObj::~BufferSeekToObj()
	{
		if (mBufferObj)
			mBufferObj->Release();
	}

	runtime::runtimeObjectBase* BufferSeekToObj::doCall(doCallContext *context)
	{
		auto r = uintObject::CreateUintObject(-1);
		if (context->GetParamCount() != 1)
			return r;

		r->mVal = mBufferObj->mPosition;
		mBufferObj->mPosition = std::min(context->GetUint32Param(0), (uint32_t)mBufferObj->mBuffer.size());
		return r;
	}

	////////////////////////////////////////////////////////////////////////////

	BufferWriteObj::BufferWriteObj()
		: mWriteType(BRT_INT32)
		, mBufferObj(nullptr)
	{
	}

	BufferWriteObj::~BufferWriteObj()
	{
		if (mBufferObj)
			mBufferObj->Release();
	}

	runtime::runtimeObjectBase* BufferWriteObj::doCall(doCallContext *context)
	{
		switch (mWriteType)
		{
		case BRT_INT32:
			do {
				if (context->GetParamCount() != 1)
					break;
				if (mBufferObj->mPosition + 4 > mBufferObj->mBuffer.size())
					mBufferObj->mBuffer.resize(mBufferObj->mPosition + 4);
				int32_t v = context->GetInt32Param(0);
				memcpy(&mBufferObj->mBuffer[0] + mBufferObj->mPosition, &v, 4);
				mBufferObj->mPosition += 4;
				return intObject::CreateIntObject(4);
			} while (0);
			return intObject::CreateIntObject(-1);

		case BRT_UINT32:
			do {
				if (context->GetParamCount() != 1)
					break;
				if (mBufferObj->mPosition + 4 > mBufferObj->mBuffer.size())
					mBufferObj->mBuffer.resize(mBufferObj->mPosition + 4);
				uint32_t v = context->GetUint32Param(0);
				memcpy(&mBufferObj->mBuffer[0] + mBufferObj->mPosition, &v, 4);
				mBufferObj->mPosition += 4;
				return intObject::CreateIntObject(4);
			} while (0);
			return intObject::CreateIntObject(-1);

		case BRT_STRING:
			do {
				// 第一个参数是等待写入的串（字符串）
				// 第二个参数是写入字节数
				// 如果写入字节数>字符串长度，需填充0
				// 如果写入字节数<字符串长度，则截断，且最后一个写入字节为0

				auto paramCount = context->GetParamCount();
				if (paramCount != 2)
					break;
				uint32_t len = 0;
				const char *str = context->GetStringParam(0, len);
				uint32_t toWriteLen = context->GetUint32Param(1);
				if (mBufferObj->mPosition + toWriteLen > mBufferObj->mBuffer.size())
					mBufferObj->mBuffer.resize(mBufferObj->mPosition + toWriteLen);
				uint32_t realWrite = std::min(toWriteLen, len);
				memcpy(&mBufferObj->mBuffer[0] + mBufferObj->mPosition, str, realWrite);
				if (realWrite > toWriteLen)
					memset(&mBufferObj->mBuffer[0] + mBufferObj->mPosition + realWrite, 0, toWriteLen - realWrite);
				mBufferObj->mPosition += toWriteLen;
			} while (0);
			return intObject::CreateIntObject(-1);

		case BRT_INT16:
			do {
				if (context->GetParamCount() != 1)
					break;
				if (mBufferObj->mPosition + 2 > mBufferObj->mBuffer.size())
					mBufferObj->mBuffer.resize(mBufferObj->mPosition + 2);
				int32_t v = context->GetInt32Param(0);
				int16_t temp = abs(v);
				if (v < 0)
					temp *= -1;
				memcpy(&mBufferObj->mBuffer[0] + mBufferObj->mPosition, &temp, 2);
				mBufferObj->mPosition += 2;
				return intObject::CreateIntObject(2);
			} while (0);
			return intObject::CreateIntObject(-1);

		case BRT_UINT16:
			do {
				if (context->GetParamCount() != 1)
					break;
				if (mBufferObj->mPosition + 2 > mBufferObj->mBuffer.size())
					mBufferObj->mBuffer.resize(mBufferObj->mPosition + 2);
				uint32_t v = context->GetUint32Param(0);
				uint16_t temp = static_cast<uint16_t>(v);
				memcpy(&mBufferObj->mBuffer[0] + mBufferObj->mPosition, &temp, 2);
				mBufferObj->mPosition += 2;
				return intObject::CreateIntObject(2);
			} while (0);
			return intObject::CreateIntObject(-1);

		case BRT_UINT8:
			do {
				if (context->GetParamCount() != 1)
					break;
				if (mBufferObj->mPosition + 1 > mBufferObj->mBuffer.size())
					mBufferObj->mBuffer.resize(mBufferObj->mPosition + 1);
				uint32_t v = context->GetUint32Param(0);
				uint8_t temp = static_cast<uint8_t>(v);
				memcpy(&mBufferObj->mBuffer[0] + mBufferObj->mPosition, &temp, 1);
				mBufferObj->mPosition += 1;
				return intObject::CreateIntObject(1);
			} while (0);
			return intObject::CreateIntObject(-1);
		}
		return intObject::CreateIntObject(-1);
	}

	////////////////////////////////////////////////////////////////////////////

	BufferReadObj::BufferReadObj()
		: mReadType(BRT_INT32)
		, mBufferObj(nullptr)
	{
	}

	BufferReadObj::~BufferReadObj()
	{
		if (mBufferObj)
			mBufferObj->Release();
	}

	runtime::runtimeObjectBase* BufferReadObj::doCall(doCallContext *context)
	{
		switch (mReadType)
		{
		case BRT_INT32:
			do {
				if (mBufferObj->mPosition + 4 > mBufferObj->mBuffer.size())
					return intObject::CreateIntObject(0);
				auto *r = intObject::CreateIntObject(*reinterpret_cast<const int*>(mBufferObj->mBuffer.c_str() + mBufferObj->mPosition));
				mBufferObj->mPosition += 4;
				return r;
			} while (0);

		case BRT_UINT32:
			do {
				if (mBufferObj->mPosition + 4 > mBufferObj->mBuffer.size())
					return uintObject::CreateUintObject(0);
				auto *r = uintObject::CreateUintObject(*reinterpret_cast<const uint32_t*>(mBufferObj->mBuffer.c_str() + mBufferObj->mPosition));
				mBufferObj->mPosition += 4;
				return r;
			} while (0);

		case BRT_STRING:
			do {
				auto *r = new ObjectModule<stringObject>;
				if (context->GetParamCount() != 1)
					return r;
				auto len = context->GetUint32Param(0);
				if (mBufferObj->mPosition + len > mBufferObj->mBuffer.size())
					return r;
				r->mVal->append(mBufferObj->mBuffer.c_str() + mBufferObj->mPosition, len);
				(*r->mVal)[len - 1] = 0;
				r->mVal->resize(strlen(r->mVal->c_str()));
				return r;
			} while (0);

		case BRT_UINT16:
			do {
				if (mBufferObj->mPosition + 2 > mBufferObj->mBuffer.size())
					return uintObject::CreateUintObject(0);
				auto *r = uintObject::CreateUintObject(*reinterpret_cast<const uint16_t*>(mBufferObj->mBuffer.c_str() + mBufferObj->mPosition));
				mBufferObj->mPosition += 2;
				return r;
			} while (0);

		case BRT_UINT8:
			do {
				if (mBufferObj->mPosition + 1 > mBufferObj->mBuffer.size())
					return uintObject::CreateUintObject(0);
				auto *r = uintObject::CreateUintObject(*reinterpret_cast<const uint8_t*>(mBufferObj->mBuffer.c_str() + mBufferObj->mPosition));
				mBufferObj->mPosition++;
				return r;
			} while (0);
		}
		return NullTypeObject::CreateNullTypeObject();
	}

	////////////////////////////////////////////////////////////////////////////

	BufferObject::BufferObject()
		: mPosition(0)
	{
	}

	uint32_t BufferObject::GetObjectTypeId() const
	{
		return DT_buffer;
	}

	stringObject* BufferObject::toString()
	{
		auto *r = new ObjectModule<stringObject>;
		*r->mVal = mBuffer;
		return r;
	}

	runtimeObjectBase* BufferObject::GetMember(const char *memName)
	{
		// 移动缓冲指针到指定位置，同时返回移动前的位置
		// 返回0xFFFFFFFF表示失败
		if (!strcmp("SeekTo", memName))
		{
			auto *r = new ObjectModule<BufferSeekToObj>;
			r->mBufferObj = this;
			AddRef();
			return r;
		}
		else if (!strcmp("Position", memName))
		{
			return uintObject::CreateUintObject(mPosition);
		}
		else if (!strcmp("ReadInt32", memName))
		{
			auto *r = new ObjectModule<BufferReadObj>;
			r->mBufferObj = this;
			r->mReadType = BRT_INT32;
			AddRef();
			return r;
		}
		else if (!strcmp("ReadUint32", memName))
		{
			auto *r = new ObjectModule<BufferReadObj>;
			r->mBufferObj = this;
			r->mReadType = BRT_UINT32;
			AddRef();
			return r;
		}
		else if (!strcmp("ReadString", memName))
		{
			// 从缓冲的当前位置读取指定数量字节，并从第一个0开始截断赋值到返回值中去
			// 读取的数据中最后一个字节若非0，则置为0
			auto *r = new ObjectModule<BufferReadObj>;
			r->mBufferObj = this;
			r->mReadType = BRT_STRING;
			AddRef();
			return r;
		}
		else if (!strcmp("ReadUint8", memName))
		{
			auto *r = new ObjectModule<BufferReadObj>;
			r->mBufferObj = this;
			r->mReadType = BRT_UINT8;
			AddRef();
			return r;
		}
		else if (!strcmp("ReadUint16", memName))
		{
			auto *r = new ObjectModule<BufferReadObj>;
			r->mBufferObj = this;
			r->mReadType = BRT_UINT16;
			AddRef();
			return r;
		}
		else if (!strcmp("WriteInt32", memName))
		{
			auto *r = new ObjectModule<BufferReadObj>;
			r->mBufferObj = this;
			r->mReadType = BRT_INT32;
			AddRef();
			return r;
		}
		else if (!strcmp("WriteUint32", memName))
		{
			auto *r = new ObjectModule<BufferWriteObj>;
			r->mBufferObj = this;
			r->mWriteType = BRT_UINT32;
			AddRef();
			return r;
		}
		else if (!strcmp("WriteString", memName))
		{
			// 在缓冲当前位置写入指定字节，如果指定的字串长度大于指定字节，
			// 则截断写入的内容，且保证最后写入的是字节0
			auto *r = new ObjectModule<BufferWriteObj>;
			r->mBufferObj = this;
			r->mWriteType = BRT_STRING;
			AddRef();
			return r;
		}
		else if (!strcmp("WriteInt16", memName))
		{
			auto *r = new ObjectModule<BufferWriteObj>;
			r->mBufferObj = this;
			r->mWriteType = BRT_INT16;
			AddRef();
			return r;
		}
		else if (!strcmp("WriteUint16", memName))
		{
			auto *r = new ObjectModule<BufferWriteObj>;
			r->mBufferObj = this;
			r->mWriteType = BRT_UINT16;
			AddRef();
			return r;
		}
		else if (!strcmp("WriteUint8", memName))
		{
			auto *r = new ObjectModule<BufferWriteObj>;
			r->mBufferObj = this;
			r->mWriteType = BRT_UINT8;
			AddRef();
			return r;
		}
		else if (!strcmp("Length", memName))
		{
			return uintObject::CreateUintObject(mBuffer.size());
		}
		else if (!strcmp("ReadLine", memName))
		{
			auto *r = new ObjectModule<BufferReadLineObj>;
			r->mBufferObj = this;
			AddRef();
			return r;
		}
		else if (!strcmp("Truncate", memName))
		{
			auto *r = new ObjectModule<BufferTruncateObj>;
			r->mBufferObj = this;
			AddRef();
			return r;
		}
		return runtime::baseTypeObject::GetMember(memName);
	}
}
