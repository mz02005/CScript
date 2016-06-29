#include "stdafx.h"
#include "xmlparserHelper.h"

using namespace notstd;

static xmlSavingTreeNode make_tree_node(bool e, XmlElemSerializerBase *node)
{
	xmlSavingTreeNode n = { e, node };
	return n;
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_BASETYPE_XMLPROP(bool)
IMPLEMENT_BASETYPE_XMLPROP(int)
//IMPLEMENT_BASETYPE_XMLPROP(uint64_t)
IMPLEMENT_BASETYPE_XMLPROP(string)
IMPLEMENT_BASETYPE_XMLPROP(uint16_t)
IMPLEMENT_BASETYPE_XMLPROP(uint8_t)
IMPLEMENT_BASETYPE_XMLPROP(uint32_t)

///////////////////////////////////////////////////////////////////////////////

XmlPropSerializerBase::XmlPropSerializerBase()
	: mParent(NULL)
{
}

std::string XmlPropSerializerBase::ToString() const
{
	throw std::bad_cast();
	return "";
}

bool XmlPropSerializerBase::FromString(const char *s)
{
	throw std::bad_cast();
	return false;
}

void XmlPropSerializerBase::Normalize(XmlElemSerializerBase *parent)
{
}

IMPLEMENT_XMLSERIAL_PROP(XmlPropSerializerBase, objBase)

///////////////////////////////////////////////////////////////////////////////

XmlFlagItem::XmlFlagItem(ItemType it, const char *n, size_t o, 
	const char *ltn, const char *dv)
	: itemType(it)
	, name(n)
	, dataOff(o)
	, listTagName(ltn)
	, defValue(dv)
{
}

///////////////////////////////////////////////////////////////////////////////

XmlElemSerializerBase::XmlElemSerializerBase()
	: mParent(NULL)
	, mHasCharactors(false)
{
}

void XmlElemSerializerBase::Normalize(XmlElemSerializerBase *parent)
{
}

IMPLEMENT_XMLSERIAL_ELEM(XmlElemSerializerBase, objBase)

bool XmlElemSerializerBase::GetXmlFlagItemList(XmlFlagItem::ItemType type, List<const XmlFlagItem*> &flagItemList) const
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////

SubElementList::XmlSubElementListType& SubElementList::GetList()
{
	return mSubElemList;
}

IMPLEMENT_OBJINFO(SubElementList, objBase)

///////////////////////////////////////////////////////////////////////////////

bool XmlElemSerializer::WriteElemPropList(XmlElemSerializerBase *elem, xmlTextWriterPtr tWriter)
{
	List<const XmlFlagItem*> xmlFlagItemList;
	size_t count = 0;

	if (!elem->GetXmlFlagItemList(XmlFlagItem::PropertyItem, xmlFlagItemList))
		return false;

	POSITION pos = xmlFlagItemList.GetHeadPosition();
	while (pos)
	{
		const XmlFlagItem *flag = xmlFlagItemList.GetNext(pos);
		XmlPropSerializerBase *propBase = reinterpret_cast<XmlPropSerializerBase*>(reinterpret_cast<char*>(elem)+flag->dataOff);
		::xmlTextWriterWriteAttribute(tWriter, reinterpret_cast<const xmlChar*>(flag->name),
			reinterpret_cast<const xmlChar*>(propBase->ToString().c_str()));
	}
	return true;
}

bool XmlElemSerializer::WriteElemStart(XmlElemSerializerBase *elem, xmlTextWriterPtr tWriter)
{
	::xmlTextWriterStartElement(tWriter, reinterpret_cast<const xmlChar*>(elem->GetTagName().c_str()));
	WriteElemPropList(elem, tWriter);
	return true;
}

bool XmlElemSerializer::WriteElemEnd(XmlElemSerializerBase *elem, xmlTextWriterPtr tWriter)
{
	if (elem->mHasCharactors)
		::xmlTextWriterWriteString(tWriter, reinterpret_cast<const xmlChar*>(elem->mCharacters.c_str()));
	::xmlTextWriterEndElement(tWriter);
	return true;
}

void XmlElemSerializer::AddSubNodeHelper(List<xmlSavingTreeNode> &tnList, XmlElemSerializerBase *elem)
{
	List<const XmlFlagItem*> xmlFlagItemList;

	if (!elem->GetXmlFlagItemList(XmlFlagItem::SubElementItem, xmlFlagItemList))
		return;
	POSITION pos = xmlFlagItemList.GetTailPosition();
	while (pos)
	{
		const XmlFlagItem *flag = xmlFlagItemList.GetPrev(pos);
		tnList.AddHead(make_tree_node(false, reinterpret_cast<XmlElemSerializerBase*>(reinterpret_cast<char*>(elem)+flag->dataOff)));
	}

	// 保存列表元素
	xmlFlagItemList.RemoveAll();
	if (!elem->GetXmlFlagItemList(XmlFlagItem::SubListItem, xmlFlagItemList))
		return;
	pos = xmlFlagItemList.GetHeadPosition();
	while (pos)
	{
		const XmlFlagItem *theNode = xmlFlagItemList.GetNext(pos);
		SubElementList *subElemList = reinterpret_cast<SubElementList*>(
			reinterpret_cast<char*>(elem)+theNode->dataOff);
		auto oneSubList = subElemList->GetList();
		POSITION subPos = oneSubList.GetTailPosition();
		while (subPos)
		{
			auto &theBase = oneSubList.GetPrev(subPos);
			assert(theBase->GetTagName() == (theNode->listTagName ? theNode->listTagName : theNode->name));
			tnList.AddHead(make_tree_node(false, theBase));
		}
	}
}

XmlElemSerializer::XmlElemSerializer(const std::string &path)
	: mPath(path)
{
}

bool XmlElemSerializer::Store(XmlElemSerializerBase *root)
{
	if (!root)
		return false;

	xmlTextWriterPtr tWriter = ::xmlNewTextWriterFilename(mPath.c_str(), 0);
	::xmlTextWriterSetIndent(tWriter, 1);
	bool hasDoc = false;
	do {
		if (!tWriter)
			break;
		if (xmlTextWriterStartDocument(tWriter, "1.0", "utf-8", NULL))
			break;
		hasDoc = true;

		List<xmlSavingTreeNode> tnList;
		AddSubNodeHelper(tnList, root);

		// 开始root
		WriteElemStart(root, tWriter);
		while (!tnList.IsEmpty())
		{
			xmlSavingTreeNode tn = tnList.GetHead();
			tnList.RemoveHead();

			if (tn.isEnd)
				WriteElemEnd(tn.node, tWriter);
			else
			{
				WriteElemStart(tn.node, tWriter);
				tnList.AddHead(make_tree_node(true, tn.node));
				AddSubNodeHelper(tnList, tn.node);
			}
		}
		// 结束root
		WriteElemEnd(root, tWriter);
	} while (0);
	if (hasDoc)
		::xmlTextWriterEndDocument(tWriter);
	if (tWriter)
		::xmlFreeTextWriter(tWriter);

	return true;
}

class XmlReader : public xmlSAXParser
{
private:
	struct LevelInfo
	{
		bool isList;
		XmlElemSerializerBase *base;

		LevelInfo(bool b, XmlElemSerializerBase *s)
			: isList(b)
			, base(s)
		{
		}
	};
	bool mHasStartRoot;
	std::list<LevelInfo> mParseQueue;

	void ThrowExeception(const char *err)
	{
#if defined(PLATFORM_WINDOWS)
		throw std::exception("Invalid root element.");
#else
		throw std::exception();
#endif
	}

protected:
	virtual void OnStartElement(const char *name, const char **atts)
	{
		List<const XmlFlagItem*> flags;

		if (!mHasStartRoot)
		{
			if (mParseQueue.front().base->GetTagName() != name)
				ThrowExeception("Invalid root element");
			mHasStartRoot = true;
		}
		else
		{
			mParseQueue.front().base->GetXmlFlagItemList(XmlFlagItem::NullType, flags);
			auto iter = flags.GetHeadPosition();
			while (iter)
				//for (; iter != flags.end(); iter++)
			{
				auto &theBase = flags.GetNext(iter);

				if (theBase->itemType == XmlFlagItem::SubElementItem)
				{
					if (strcmp(theBase->name, name))
						continue;
					XmlElemSerializerBase *base = reinterpret_cast<XmlElemSerializerBase*>(reinterpret_cast<char*>(mParseQueue.front().base) + theBase->dataOff);
					base->mCharacters.clear();
					mParseQueue.push_front(LevelInfo(false, base));
					break;
				}
				else if (theBase->itemType == XmlFlagItem::SubListItem)
				{
					if (strcmp(theBase->listTagName ? theBase->listTagName : theBase->name, name))
						continue;
					SubElementList *elemList = reinterpret_cast<SubElementList*>(reinterpret_cast<char*>(mParseQueue.front().base) + theBase->dataOff);
					objBase *obj = objBase::CreateObject(theBase->name);
					if (!obj->isInheritFrom(OBJECT_INFO(XmlElemSerializerBase)))
						throw std::bad_cast();
					elemList->GetList().AddTail(static_cast<XmlElemSerializerBase*>(obj));
					mParseQueue.push_front(LevelInfo(true, static_cast<XmlElemSerializerBase*>(obj)));
					break;
				}
			}
		}

		if (atts)
		{
			flags.RemoveAll();
			XmlElemSerializerBase *elemBase = mParseQueue.front().base;
			elemBase->GetXmlFlagItemList(XmlFlagItem::PropertyItem, flags);
			for (const char **v = atts; *v != '\0'; v += 2)
			{
				auto iter = flags.GetHeadPosition();
				while (iter)
					//for (; iter != flags.end(); iter++)
				{
					auto &theBase = flags.GetNext(iter);
					if (strcmp(theBase->name, *v) == 0)
					{
						XmlPropSerializerBase *propBase = reinterpret_cast<XmlPropSerializerBase*>(reinterpret_cast<char*>(mParseQueue.front().base) + theBase->dataOff);
						propBase->FromString(v[1]);
						propBase->Normalize(elemBase);
						break;
					}
				}
			}
		}
	}

	virtual void OnEndElement(const char *name)
	{
		XmlElemSerializerBase *base = mParseQueue.front().base;
		if (base->GetTagName() == name)
		{
			mParseQueue.pop_front();
			base->Normalize(mParseQueue.size() ? mParseQueue.front().base : NULL);
		}
	}

	virtual void OnCharacters(const char *ch, int len)
	{
		XmlElemSerializerBase *base = mParseQueue.front().base;
		if (base->GetTagName() == mNameQueue->back())
		{
			std::string s(ch, len);
			base->mCharacters += s;
		}
	}

public:
	XmlReader(XmlElemSerializerBase *root)
		: mHasStartRoot(false)
	{
		mParseQueue.push_front(LevelInfo(false, root));
	}
};

void XmlElemSerializer::Load(XmlElemSerializerBase *root) throw()
{
	InitializeDOMTree(root);
	XmlReader reader(root);
	xmlKeepBlanksDefault(0);
	reader.ParseFile(mPath);
}

struct TravRecord
{
	// 已经将众儿子加进列表了
	int hasTrav : 1;
	int isList : 1;

	XmlElemSerializerBase *base;
	SubElementList *subList;

	TravRecord(XmlElemSerializerBase *b)
		: hasTrav(0)
		, isList(0)
		, base(b)
	{
	}

	TravRecord(SubElementList *l)
		: hasTrav(0)
		, isList(1)
		, subList(l)
	{
	}
};

void XmlElemSerializer::DFSTheElemTree(XmlElemSerializerBase *root,
	std::function<void(notstd::SubElementList*)> funcSubList,
	std::function<bool(XmlElemSerializerBase*)> func,
	std::function<void(XmlPropSerializerBase*, const XmlFlagItem *)> funcProp)
{
	std::list<TravRecord> elemList;
	elemList.push_back(TravRecord(root));

	while (!elemList.empty())
	{
		TravRecord &r = elemList.back();
		if (!r.hasTrav)
		{
			r.hasTrav = true;

			if (r.isList)
			{
				SubElementList::XmlSubElementListType &sl = r.subList->GetList();
				POSITION slPos = sl.GetHeadPosition();
				while (slPos)
				{
					XmlElemSerializerBase *elem = sl.GetNext(slPos);
					elemList.push_back(TravRecord(elem));
				}
			}
			else
			{
				List<const XmlFlagItem *> flagList;
				r.base->GetXmlFlagItemList(XmlFlagItem::NullType, flagList);
				POSITION flagPos = flagList.GetHeadPosition();
				while (flagPos)
				{
					const XmlFlagItem *flag = flagList.GetNext(flagPos);
					if (flag->itemType == XmlFlagItem::PropertyItem)
					{
						XmlPropSerializerBase *prop = reinterpret_cast<XmlPropSerializerBase*>(
							reinterpret_cast<char*>(r.base) + flag->dataOff);
						prop->mParent = r.base;
						funcProp(prop, flag);
					}
					else if (flag->itemType == XmlFlagItem::SubElementItem)
					{
						XmlElemSerializerBase *elem = reinterpret_cast<XmlElemSerializerBase*>(
							reinterpret_cast<char*>(r.base) + flag->dataOff);
						elem->mParent = r.base;
						elemList.push_back(TravRecord(elem));
					}
					else if (flag->itemType == XmlFlagItem::SubListItem)
					{
						SubElementList *subElemList = reinterpret_cast<SubElementList*>(
							reinterpret_cast<char*>(r.base) + flag->dataOff);
						elemList.push_back(TravRecord(subElemList));
					}
				}
			}
		}
		else
		{
			if (r.isList)
			{
				funcSubList(r.subList);
			}
			else
			{
				func(r.base);
			}
			elemList.pop_back();
		}
	}
}

void XmlElemSerializer::TravElemTreeByLevel(XmlElemSerializerBase *root,
	std::function<void(notstd::SubElementList*)> funcSubList,
	std::function<bool(XmlElemSerializerBase*)> func,
	std::function<void(XmlPropSerializerBase*, const XmlFlagItem *)> funcProp)
{
	std::list<XmlElemSerializerBase*> elemList;
	elemList.push_back(root);

	while (!elemList.empty())
	{
		XmlElemSerializerBase *p = elemList.front();
		elemList.pop_front();

		// 返回非0表示不要处理子项
		if (!func(p))
			continue;

		List<const XmlFlagItem*> xmlFlagItemList;
		p->GetXmlFlagItemList(XmlFlagItem::NullType, xmlFlagItemList);
		POSITION pos = xmlFlagItemList.GetHeadPosition();
		while (pos)
		//std::for_each(xmlFlagItemList.begin(), xmlFlagItemList.end(),
		//	[p, &funcSubList, &elemList, &funcProp](const XmlFlagItem *flag)
		{
			const auto *flag = xmlFlagItemList.GetNext(pos);
			if (flag->itemType == XmlFlagItem::SubElementItem)
			{
				XmlElemSerializerBase *e = reinterpret_cast<XmlElemSerializerBase*>(
					reinterpret_cast<char*>(p)+flag->dataOff);
				e->mParent = p;
				elemList.push_back(e);
			}
			else if (flag->itemType == XmlFlagItem::SubListItem)
			{
				SubElementList *subElemList = reinterpret_cast<SubElementList*>(
					reinterpret_cast<char*>(p)+flag->dataOff);
				funcSubList(subElemList);
				auto theSubList = subElemList->GetList();
				auto subListPos = theSubList.GetHeadPosition();
				//std::for_each(theSubList.begin(), theSubList.end(),
				//	[&](XmlElemSerializerBase *sl)
				while (subListPos)
				{
					XmlElemSerializerBase *sl = theSubList.GetNext(subListPos);
					sl->mParent = p;
					elemList.push_back(sl);
				}
			}
			else if (flag->itemType == XmlFlagItem::PropertyItem)
			{
				XmlPropSerializerBase *prop = reinterpret_cast<XmlPropSerializerBase*>(
					reinterpret_cast<char*>(p)+flag->dataOff);
				prop->mParent = p;
				funcProp(prop, flag);
			}
		}
		//);
	}
}

void XmlElemSerializer::ReleaseXmlDomTree(XmlElemSerializerBase *root)
{
#if defined(PLATFORM_WINDOWS)
	DFSTheElemTree(root, [](notstd::SubElementList *sl)
	{
		POSITION pos = sl->GetList().GetHeadPosition();
		while (pos)
		{
			XmlElemSerializerBase *theListItem = sl->GetList().GetNext(pos);
			(*theListItem->GetThisObjInfo()->DestroyObject)(theListItem);
		}
		sl->GetList().RemoveAll();
	}, [](notstd::XmlElemSerializerBase *elem)->bool
	{ return true; }, [](notstd::XmlPropSerializerBase *prop, const XmlFlagItem *flag){});
#else
	assert(0);
#endif
}

void XmlElemSerializer::InitializeDOMTree(XmlElemSerializerBase *base)
{
#if defined(PLATFORM_WINDOWS)
	DFSTheElemTree(base,
		[](notstd::SubElementList *sl)
		{
			POSITION pos = sl->GetList().GetHeadPosition();
			while (pos)
			{
				XmlElemSerializerBase *theListItem = sl->GetList().GetNext(pos);
				(*theListItem->GetThisObjInfo()->DestroyObject)(theListItem);
			}
			sl->GetList().RemoveAll(); 
		},

		[](notstd::XmlElemSerializerBase *elem)->bool{ return true; },

		[](notstd::XmlPropSerializerBase *prop, const XmlFlagItem *flag){
			if (flag->defValue)
				prop->FromString(flag->defValue);
		}
	);
#else
	assert(0);
#endif
}
