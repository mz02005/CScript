#include "stdafx.h"
#include "svnTool.h"
#include "notstd/notstd.h"

namespace tools {
	class svnGetRevisionObj : public runtime::baseTypeObject
	{
	public:
		virtual runtime::runtimeObjectBase* doCall(runtime::doCallContext *context)
		{
			runtime::runtimeObjectBase *def = runtime::NullTypeObject::CreateNullTypeObject();
			runtime::stringObject *r = new runtime::ObjectModule<runtime::stringObject>;

			if (context->GetParamCount() < 1)
				return def;
			const char *path = context->GetStringParam(0);
			if (!path)
				return def;

			static const char svnResultFileTempName[] = "___temp___.txt";
			std::string cmdline = "svn info ";
			cmdline += path;
			cmdline.append(" > ").append(svnResultFileTempName);
			if (::system(cmdline.c_str()) < 0)
				return def;

			Handle<STDCFILEHandle> file = ::fopen(svnResultFileTempName, "rb");
			if (!file)
				return def;
			bool findIt = false;
			notstd::StringHelper::ReadLineContextT<4096> c;
			std::string line;
			while (notstd::StringHelper::ReadLine(file, line, &c))
			{
				std::string::size_type f;
				static const char toFind[] = "Revision:";
				static const size_t fl = strlen(toFind);
				if ((f = line.find(toFind)) != line.npos)
				{
					std::string revision = notstd::StringHelper::Mid(line, f + fl + 1);
					notstd::StringHelper::Trim(revision, "\t ");
					*r->mVal = revision;
					findIt = true;
					break;
				}
			}
			file.Close();
			::remove(svnResultFileTempName);
			if (findIt)
				delete def;
			return findIt ? r : def;
		}
	};

	///////////////////////////////////////////////////////////////////////////

	runtime::runtimeObjectBase* svnTools::GetMember(const char *memName)
	{
		if (!strcmp("getRevision", memName))
		{
			// 静态成员，不需要保存父对象
			return new runtime::ObjectModule<svnGetRevisionObj>;
		}
		return runtime::baseObjDefault::GetMember(memName);
	}
}
