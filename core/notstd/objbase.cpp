#include "stdafx.h"
#include "objbase.h"
#include <assert.h>

const ObjInfo objBase::mObjInfo = 
{
	"objBase",
	NULL,
	NULL,
	NULL,
	NULL,
};
const ObjInfo* objBase::mTempInfo = NULL;
const ObjInfo* objBase::mFirstObjInfo = NULL;

objBase::ObjInfoTable* objBase::mObjInfoTable = NULL;

void objBase::RegistAllClass()
{
	assert(mObjInfoTable == NULL);
	mObjInfoTable = new objBase::ObjInfoTable;
	const ObjInfo *p = objBase::mFirstObjInfo;
	while (p)
	{
		(*mObjInfoTable)[p->className] = p;
		p = p->nextClass;
	}
}

void objBase::UnregistAllClass()
{
	if (mObjInfoTable) {
		delete mObjInfoTable;
		mObjInfoTable = NULL;
	}
}

objBase* objBase::CreateObject(const char *className)
{
	ObjInfoTable::const_iterator it = mObjInfoTable->find(className);
	if (it == mObjInfoTable->end())
		return NULL;
	return (*it->second->CreateObject)();
}

void objBase::DestroyObject(objBase *obj)
{
	assert(obj->GetThisObjInfo()->DestroyObject);
	(*obj->GetThisObjInfo()->DestroyObject)(obj);
}

objBase::~objBase()
{
}

bool objBase::isInheritFrom(const ObjInfo *objInfo) const
{
	const ObjInfo *myObjInfo = GetThisObjInfo();
	return isInheritFrom(myObjInfo, objInfo);
}

bool objBase::isInheritFrom(const ObjInfo *objInfo, const ObjInfo *from)
{
	while (objInfo && objInfo != from)
		objInfo = objInfo->parentClass;
	return (objInfo == from);
}
