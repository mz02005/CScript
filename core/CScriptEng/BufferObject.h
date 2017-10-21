#pragma once
#include "rtTypes.h"
#include <string.h>

namespace runtime {

	class BufferObject;
	class TCPSocketSendObj;
	class StringCodecObj;

	class BufferTruncateObj : public runtime::baseObjDefault
	{
		friend class BufferObject;

	private:
		BufferObject *mBufferObj;

	public:
		BufferTruncateObj();
		virtual ~BufferTruncateObj();
		virtual runtime::runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class BufferReadLineObj : public runtime::baseObjDefault
	{
		friend class BufferObject;

	private:
		BufferObject *mBufferObj;

	public:
		BufferReadLineObj();
		virtual ~BufferReadLineObj();
		virtual runtime::runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class BufferSeekToObj : public runtime::baseObjDefault
	{
		friend class BufferObject;

	private:
		BufferObject *mBufferObj;

	public:
		BufferSeekToObj();
		virtual ~BufferSeekToObj();
		virtual runtime::runtimeObjectBase* doCall(doCallContext *context) override;
	};

	enum {
		BRT_INT32,
		BRT_UINT32,
		BRT_INT16,
		BRT_UINT16,
		BRT_STRING,
		BRT_UINT8,
	};

	class BufferWriteObj : public runtime::baseObjDefault
	{
		friend class BufferObject;

	private:
		int mWriteType;
		BufferObject *mBufferObj;

	public:
		BufferWriteObj();
		virtual ~BufferWriteObj();
		virtual runtime::runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class BufferReadObj : public runtime::baseObjDefault
	{
		friend class BufferObject;

	private:
		int mReadType;
		BufferObject *mBufferObj;

	public:
		BufferReadObj();
		virtual ~BufferReadObj();
		virtual runtime::runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class BufferObject : public runtime::baseTypeObject
	{
		friend class StringCodecObj;

		friend class CreateBufferObj;
		friend class BufferSeekToObj;
		friend class BufferReadObj;
		friend class BufferWriteObj;
		friend class TCPSocketSendObj;
		friend class TCPSocketReceiveObj;
		friend class BufferReadLineObj;
		friend class BufferTruncateObj;

	private:
		std::string mBuffer;
		uint32_t mPosition;

	public:
		BufferObject();
		virtual uint32_t GetObjectTypeId() const override;
		virtual stringObject* toString() override;
		virtual runtimeObjectBase* GetMember(const char *memName) override;
	};
}
