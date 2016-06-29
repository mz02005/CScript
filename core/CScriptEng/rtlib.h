#pragma once
#include "rtTypes.h"
#include "compile.h"

namespace runtime {
	class sleepObj : public runtime::baseObjDefault
	{
	public:
		sleepObj();
		virtual uint32_t GetObjectTypeId() const;
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class rtLibHelper
	{
	public:
		static bool RegistObjNames(compiler::FunctionStatement *sb);
		static bool RegistRuntimeObjs(runtimeContext *context);
	};

	// 用于打印对象的对象
	class printObj : public runtime::baseObjDefault
	{
	private:
		bool mPrintLine;

	public:
		printObj();

		void SetIsPrintLine(bool isPrintLine);

		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	// 随机数函数
	class randObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class srandObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	// sin函数
	class sinObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};
	
	class powfObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	// 获取时间
	class timeObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class systemCallObject : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class unzipFileObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class zipFilesInDirectoryObj : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};

	class csOpenFile : public runtime::baseObjDefault
	{
	public:
		virtual runtimeObjectBase* doCall(runtime::doCallContext *context);
	};
}
