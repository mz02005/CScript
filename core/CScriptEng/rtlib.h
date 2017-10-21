#pragma once
#include "rtTypes.h"
#include "compile.h"

namespace runtime {
	class getVersionObj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class sleepObj : public runtime::baseObjDefault
	{
	public:
		sleepObj();
		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class rtLibHelper
	{
	public:
		static bool RegistCScriptRuntimeLib(compiler::SimpleCScriptEngContext *sb);
	};

	// 用于打印对象的对象
	class printObj : public runtime::baseObjDefault
	{
	private:
		bool mPrintLine;

	public:
		printObj();

		void SetIsPrintLine(bool isPrintLine);

		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	// 随机数函数
	class randObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class srandObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	// sin函数
	class sinObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};
	
	class powfObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	// 获取时间
	class timeObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class systemCallObject : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class unzipFileObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class zipFilesInDirectoryObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class csOpenFile : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context) override;
	};

	class DeleteFileObj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class CreateBufferObj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class GetTickCountObj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class ParseInt32Obj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class ParseUint32Obj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class tcpConnectObj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class inet_ntoaObj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class AddressFromHostnameObj : public baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};

	class NetworkByteOrderToHostByteOrderObj : public baseObjDefault
	{
	public:
		enum ByteOrderType {
			BO_UINT16,
			BO_UINT32,
			BO_FLOATFROMINT32,
			BO_DOUBLEFROMINT64,
		};

	private:
		ByteOrderType mType;

	public:
		static NetworkByteOrderToHostByteOrderObj* Create(ByteOrderType t);
		virtual runtimeObjectBase* doCall(doCallContext *context) override;
	};
}
