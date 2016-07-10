#ifndef NOTSTD_OBJBASE_H
#define NOTSTD_OBJBASE_H

#pragma once
#include <map>
#include "config.h"

namespace notstd {

	class objBase;

	struct ObjInfo {
		const char *className;
		const ObjInfo *parentClass;
		const ObjInfo *nextClass;
		objBase* (*CreateObject)();
		void(*DestroyObject)(objBase *obj);
	};

#define OBJECT_INFO(className) (&className::mObjInfo)

#define DECLARE_OBJINFO(className) \
	public: \
		const static notstd::ObjInfo mObjInfo; \
		virtual const notstd::ObjInfo* GetThisObjInfo() const;

#define DECLARE_DYNAMICOBJ(className) \
	DECLARE_OBJINFO(className) \
	public: \
		static notstd::objBase* Create##className(); \
		static void Destroy##className(notstd::objBase *obj);

#define IMPLEMENT_OBJINFO_INNER(className,parentClassName,createObjProc,destroyObjProc) \
	const notstd::ObjInfo className::mObjInfo = { \
		#className, \
		&parentClassName::mObjInfo, \
		(notstd::objBase::mTempInfo = notstd::objBase::mFirstObjInfo, notstd::objBase::mFirstObjInfo = &className::mObjInfo, objBase::mTempInfo), \
		createObjProc, destroyObjProc, \
		}; \
	const notstd::ObjInfo* className::GetThisObjInfo() const { \
		return &className::mObjInfo; \
		}

#define IMPLEMENT_OBJINFO(className,parentClassName) \
	IMPLEMENT_OBJINFO_INNER(className,parentClassName,NULL, NULL)

#define IMPLEMENT_DYNAMICOBJ(className,parentClassName) \
	IMPLEMENT_OBJINFO_INNER(className,parentClassName,&className::Create##className,&className::Destroy##className) \
	notstd::objBase* className::Create##className() { return new className; } \
	void className::Destroy##className(objBase *obj) { \
		delete obj; \
		}

	class NOTSTD_API objBase {
	public:
		const static ObjInfo *mFirstObjInfo;
		const static ObjInfo *mTempInfo;
		const static ObjInfo mObjInfo;

	private:
		typedef std::map<std::string, const ObjInfo*> ObjInfoTable;
		static ObjInfoTable *mObjInfoTable;

	public:
		virtual ~objBase();

		static void RegistAllClass();
		static void UnregistAllClass();
		static objBase* CreateObject(const char *className);
		static void DestroyObject(objBase *obj);

		virtual const ObjInfo* GetThisObjInfo() const { return &objBase::mObjInfo; }

		bool isInheritFrom(const ObjInfo *objInfo) const;
		static bool isInheritFrom(const ObjInfo *objInfo, const ObjInfo *from);
	};

}

#endif
