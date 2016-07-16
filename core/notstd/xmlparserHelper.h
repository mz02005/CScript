#pragma once
#include "config.h"
#if defined(PLATFORM_WINDOWS)
#include <d2d1.h>
#endif
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlsave.h>
#include "libxml/xmlwriter.h"
#include "notstd/objbase.h"
#include "notstd/stringHelper.h"
#include "notstd/nslist.h"
#include <string.h>

namespace notstd {
	class DataConv {
	public:
		static bool DataFromString(const char *s, int &v)
		{
			v = atoi(s);
			return true;
		}
		static bool DataFromString(const char *s, float &v)
		{
			v = static_cast<float>(atof(s));
			return true;
		}
		static bool DataFromString(const char *s, double &v)
		{
			v = atof(s);
			return true;
		}
#if defined(PLATFORM_WINDOWS)
		static bool DataFromString(const char *s, D2D1_POINT_2F &v)
		{
			int r = sscanf_s(s, "%f,%f", &v.x, &v.y);
			return (r == 2);
		}
		static bool DataFromString(const char *s, D2D1_COLOR_F &v)
		{
			int r = sscanf_s(s, "%f,%f,%f,%f", &v.r, &v.g, &v.b, &v.a);
			if (r == 3)
				v.a = 1.f;
			return (r >= 3);
		}
		static bool DataFromString(const char *s, POINT &v)
		{
			int r = sscanf_s(s, "%d,%d", &v.x, &v.y);
			return (r == 2);
		}
#endif
		static bool DataFromString(const char *s, std::string &v)
		{
			v = s;
			return true;
		}
		//static bool DataFromString(const char *s, uint64_t &v)
		//{
		//	assert(false);
		//	// 在vs2010上无法使用strtoull，待修正
		//	v = strtoul(s, NULL, 10);
		//	return true;
		//}
		static bool DataFromString(const char *s, bool &v)
		{
			if (strcmp(s, "true") == 0
				|| strcmp(s, "1") == 0)
				v = true;
			else if (strcmp(s, "false") == 0
				|| strcmp(s, "0") == 0)
				v = false;
			else
				v = false;
			return true;
		}
		static bool DataFromString(const char *s, unsigned long &v)
		{
			char *stopPos = NULL;
			v = strtoul(s, &stopPos, 10);
			return (*stopPos == 0);
		}
		static bool DataFromString(const char *s, uint32_t &v)
		{
			char *stopPos = NULL;
			v = static_cast<uint32_t>(strtoul(s, &stopPos, 10));
			return (*stopPos == 0);
		}
		static bool DataFromString(const char *s, uint16_t &v)
		{
			char *stopPos = NULL;
			v = static_cast<uint16_t>(strtoul(s, &stopPos, 10));
			return (*stopPos == 0);
		}
		static bool DataFromString(const char *s, uint8_t &v)
		{
			char *stopPos = NULL;
			v = static_cast<uint8_t>(strtoul(s, &stopPos, 10));
			return (*stopPos == 0);
		}
	};

	struct String2IntegerTableEntry
	{
		int val;
		const char *s;
	};

#define PARSE_XML_PROPERTY_STRINGTOINTEGER(isRead,node,name,v,def,s2iTable) \
	do { \
		using namespace notstd; \
		int tableSize = sizeof(s2iTable) / sizeof(s2iTable[0]); \
		const String2IntegerTableEntry *entry = s2iTable, *lastentry = s2iTable + tableSize; \
		if (!isRead) { \
					while (entry < lastentry) { \
				if (entry->val == v) { \
					notstd::xmlHelper::xmlWriteHelper::WriteProperty(node,name,entry->s); \
					break; \
								} \
				entry ++; \
								} \
			if (entry == lastentry) \
				return -1; \
				} \
				else { \
			std::string temp; \
						while (entry < lastentry) { \
				if (entry->val == def) { \
					temp = entry->s; \
					break; \
								} \
				entry ++; \
									} \
			notstd::xmlHelper::xmlReadHelper::ReadProperty(node, name, \
				temp, entry < lastentry ? temp : s2iTable[0].s); \
			entry = s2iTable; \
						while (entry < lastentry) { \
				if (entry->s == temp) { \
					v = entry->val; \
					break; \
								} \
				entry ++; \
									} \
			if (entry == lastentry) v = def; \
			} \
		} while (0)

#define PARSE_XML_PROPERTY(isRead,node,name,v,def) \
	do { \
		if (!isRead) { \
			notstd::xmlHelper::xmlWriteHelper::WriteProperty(node, name, v); \
				} \
				else { \
			if (!notstd::xmlHelper::xmlReadHelper::ReadProperty(node, name, v, def)) \
				return -1; \
			} \
		} while (0)

	class NOTSTD_API xmlSAXParser {
	protected:
		std::list<std::string> *mNameQueue;

	protected:
		virtual void OnStartElement(const char *name, const char **atts);
		virtual void OnEndElement(const char *name);
		virtual void OnCharacters(const char *ch, int len);

		virtual void BeforeParse(xmlSAXHandler *handler);
		virtual void AfterParse();

	public:
		static void onStartElement(void *ctx, const xmlChar *name, const xmlChar **atts);
		static void onEndElement(void *ctx, const xmlChar *name);
		static void onCharacters(void *ctx, const xmlChar *ch, int len);

		xmlSAXParser();
		virtual ~xmlSAXParser();

		xmlDocPtr ParseFile(const std::string &pathName);
		xmlDocPtr ParseBuffer(const char *buffer, std::size_t bufferSize);
	};

	class NOTSTD_API xmlPath {
	private:
		xmlXPathContextPtr mContext;
		xmlXPathObjectPtr mPathObject;

	public:
		xmlPath();
		~xmlPath();

		bool Find(xmlDocPtr doc, const char *strPath);
		void Close();

		xmlXPathObjectPtr GetXPathObject() {
			return mPathObject;
		}
	};

	class NOTSTD_API xmlString {
	private:
		xmlChar *mString;

	public:
		xmlString(xmlChar *str);
		xmlString();
		~xmlString();

		xmlChar** operator&() {
			return &mString;
		}

		operator const char* () {
			return reinterpret_cast<const char*>(mString);
		}

		operator std::string() {
			return std::string(mString ? reinterpret_cast<const char*>(mString) : "");
		}
	};

	class NOTSTD_API xmlDocumentHelper {
	private:
		xmlDocPtr mDoc;

	public:
		xmlDocumentHelper();
		~xmlDocumentHelper();

		bool LoadFromFile(const char *filePathName);
		bool LoadFromBuffer(const char *buffer, std::size_t size);
		void ReleaseDoc();

		operator xmlDocPtr() {
			return mDoc;
		}

		static xmlDocumentHelper* CreateDocFromFile(const char *filePathName);
		static void DestroyDocFromFile(xmlDocumentHelper *doc);
	};

	namespace xmlHelper {
		class xmlWriteHelper {
		public:
			template <class T>
			static void WriteProperty(xmlNodePtr node, const char *name, const T &t)
			{
				xmlNewProp(node, BAD_CAST name, BAD_CAST TS(t).c_str());
			}
		};

		// C++标准不支持在类实现中特化，所以这里做了修改
		class xmlReadHelper {
		public:
			template <class T>
			static bool ReadProperty(xmlNodePtr node, const char *name, T &t, const T &def);
		};

		template <class T>
		inline bool xmlReadHelper::ReadProperty(xmlNodePtr node, const char *name, T &t, const T &def)
		{
			xmlChar *r = xmlGetProp(node, BAD_CAST name);
			if (!r) {
				t = def;
				return true;
			}
			return notstd::DataConv::DataFromString(xmlString(r).operator const char*(), t);
		}

		template <>
		inline bool xmlReadHelper::ReadProperty<std::string>(xmlNodePtr node, const char *name,
			std::string &t, const std::string &def)
		{
			xmlChar *r = xmlGetProp(node, BAD_CAST name);
			if (!r)
				t = def;
			else
				t = xmlString(r).operator std::string();
			return true;
		}
	}

	namespace xmlHelper
	{
		template <typename T>
		struct xmlPropData
		{
			T &mData;
			std::string mPropName;

		public:
			xmlPropData(const char *propName, T &data)
				: mPropName(propName)
				, mData(data)
			{
			}
		};

		class xmlPropReader
		{
		private:
			xmlNodePtr mNode;

		public:
			xmlPropReader(xmlNodePtr node)
				: mNode(node)
			{
			}

			template <typename T>
			xmlPropReader& operator << (const xmlPropData<T> &propData)
			{
				notstd::xmlHelper::xmlWriteHelper::WriteProperty(mNode, propData.mPropName.c_str(), propData.mData);
				return *this;
			}

			template <typename T>
			xmlPropReader& operator >> (const xmlPropData<T> &propData)
			{
				xmlChar *r = xmlGetProp(mNode, BAD_CAST propData.mPropName.c_str());
				if (!r)
					return *this;
				notstd::DataConv::DataFromString(xmlString(r).operator const char *(), propData.mData);
				return *this;
			}
		};
	}

#pragma warning(push)
#pragma warning(disable: 4251)

	// xml对象序列化
	using namespace std;

#define DECLARE_XMLSERIAL_PROP(classname) \
	DECLARE_DYNAMICOBJ(classname)

#define IMPLEMENT_XMLSERIAL_PROP(classname,parentname) \
	IMPLEMENT_DYNAMICOBJ(classname,parentname)

#define DECLARE_XMLSERIAL_ELEM_A(classname,alias) \
	DECLARE_DYNAMICOBJ(classname) \
	public: \
	virtual bool GetXmlFlagItemList(notstd::XmlFlagItem::ItemType type, List<const notstd::XmlFlagItem*> &flagItemList) const; \
	public: \
		std::string mCharacters; \
	virtual std::string GetTagName() const \
		{ \
		return alias; \
		} 

#define DECLARE_XMLSERIAL_ELEM(classname) \
	DECLARE_DYNAMICOBJ(classname) \
	public: \
	virtual bool GetXmlFlagItemList(notstd::XmlFlagItem::ItemType type, List<const notstd::XmlFlagItem*> &flagItemList) const; \
	public: \
		std::string mCharacters; \
	virtual std::string GetTagName() const \
		{ \
	return GetThisObjInfo()->className; \
		}

#define IMPLEMENT_XMLSERIAL_ELEM(classname,parentname) \
	IMPLEMENT_DYNAMICOBJ(classname,parentname)

#define BEGIN_XMLSERIAL_FLAG_TABLE(classname) \
	bool classname::GetXmlFlagItemList(notstd::XmlFlagItem::ItemType type, List<const notstd::XmlFlagItem*> &flagItemList) const { \
		typedef classname MyClassName; \
		static const notstd::XmlFlagItem xmlFlagItemList[] = {

#define XMLSERIAL_PROP_ENTRY(propname,memname) \
			notstd::XmlFlagItem(notstd::XmlFlagItem::PropertyItem, propname, offsetof(MyClassName,memname)),

#define XMLSERIAL_PROP_ENTRY_V(propname,memname,defval) \
			notstd::XmlFlagItem(notstd::XmlFlagItem::PropertyItem, propname, offsetof(MyClassName,memname), NULL, defval),

#define XMLSERIAL_ELEM_ENTRY(classname,memname) \
			notstd::XmlFlagItem(notstd::XmlFlagItem::SubElementItem, classname, offsetof(MyClassName,memname)),

#define XMLSERIAL_SUBLIST_ENTRY(elemname,memname) \
			notstd::XmlFlagItem(notstd::XmlFlagItem::SubListItem, elemname,offsetof(MyClassName,memname)),

#define XMLSERIAL_SUBLIST_ENTRY_A(elemname,memname,alias) \
			notstd::XmlFlagItem(notstd::XmlFlagItem::SubListItem, elemname,offsetof(MyClassName,memname), alias),

#define END_XMLSERIAL_FLAG_TABLE() \
			notstd::XmlFlagItem(notstd::XmlFlagItem::NullType, NULL, 0), \
			}; \
		__super::GetXmlFlagItemList(type, flagItemList); \
		const notstd::XmlFlagItem *lastFlag = xmlFlagItemList + sizeof(xmlFlagItemList) / sizeof(xmlFlagItemList[0]) - 1; \
		for (const notstd::XmlFlagItem *flag = xmlFlagItemList; flag != lastFlag; flag++) { \
			if (type == notstd::XmlFlagItem::NullType || flag->itemType == type) \
				flagItemList.AddTail(flag); \
				} \
		return true; \
		}

#define DECLARE_BASETYPE_XMLPROP(theTypeName) \
	class NOTSTD_API theTypeName##BaseTypeXmlProp : public XmlPropSerializerBase \
		{ \
		DECLARE_XMLSERIAL_PROP(theTypeName##BaseTypeXmlProp) \
	public: \
		theTypeName mVal; \
	public: \
		virtual std::string ToString() const; \
		virtual bool FromString(const char *s); \
		};

#define IMPLEMENT_BASETYPE_XMLPROP(theTypeName) \
	IMPLEMENT_DYNAMICOBJ(theTypeName##BaseTypeXmlProp,XmlPropSerializerBase) \
	std::string theTypeName##BaseTypeXmlProp::ToString() const { \
		return TS(mVal); \
		} \
	bool theTypeName##BaseTypeXmlProp::FromString(const char *s) { \
		return notstd::DataConv::DataFromString(s, mVal); \
		}

#define DECLARE_SIMPLE_XMLELEM(classname) \
	class classname : public notstd::XmlElemSerializerBase \
		{ \
		DECLARE_XMLSERIAL_ELEM(classname) \
		\
	public: \
		classname(); \
		};

#define IMPLEMENT_SIMPLE_XMLELEM(classname) \
	IMPLEMENT_XMLSERIAL_ELEM(classname, notstd::XmlElemSerializerBase) \
	BEGIN_XMLSERIAL_FLAG_TABLE(classname) \
	END_XMLSERIAL_FLAG_TABLE() \
	classname::classname() { mHasCharactors = true; }

	///////////////////////////////////////////////////////////////////////////////

	class XmlElemSerializerBase;
	class NOTSTD_API XmlPropSerializerBase : public objBase
	{
		friend class XmlElemSerializer;
		DECLARE_XMLSERIAL_PROP(XmlPropSerializerBase)

	protected:
		XmlElemSerializerBase *mParent;

	public:
		XmlPropSerializerBase();
		virtual std::string ToString() const;
		virtual bool FromString(const char *s);
		virtual void Normalize(XmlElemSerializerBase *parent);
	};

	///////////////////////////////////////////////////////////////////////////////

	DECLARE_BASETYPE_XMLPROP(bool)
	DECLARE_BASETYPE_XMLPROP(int)
	//DECLARE_BASETYPE_XMLPROP(uint64_t)
	DECLARE_BASETYPE_XMLPROP(string)
	DECLARE_BASETYPE_XMLPROP(uint16_t)
	DECLARE_BASETYPE_XMLPROP(uint8_t)
	DECLARE_BASETYPE_XMLPROP(uint32_t)

	///////////////////////////////////////////////////////////////////////////////

	struct NOTSTD_API XmlFlagItem
	{
		enum ItemType {
			NullType,
			SubElementItem = 1,
			PropertyItem,
			SubListItem,
		} itemType;

		const char *name;
		size_t dataOff;
		const char *listTagName;
		// 指定属性的缺省值，在Initialize时，如果设定了该值，则会为属性赋初值
		const char *defValue;

		XmlFlagItem(ItemType it, const char *n, size_t o, const char *ltn = NULL, const char *dv = NULL);
	};

#if defined(PLATFORM_WINDOWS)
	template class NOTSTD_API List<const notstd::XmlFlagItem*>;
#endif

	///////////////////////////////////////////////////////////////////////////////

	class NOTSTD_API XmlElemSerializerBase : public objBase
	{
		friend class XmlElemSerializer;
		DECLARE_XMLSERIAL_ELEM(XmlElemSerializerBase)

	protected:
		bool mHasCharactors;
		XmlElemSerializerBase *mParent;

	public:
		XmlElemSerializerBase();
		virtual void Normalize(XmlElemSerializerBase *parent);
	};

#if defined(PLATFORM_WINDOWS)
	template class NOTSTD_API List<XmlElemSerializerBase*>;
#endif

	///////////////////////////////////////////////////////////////////////////////

	class NOTSTD_API SubElementList : public objBase
	{
		DECLARE_OBJINFO(SubElementList)

	public:
		typedef List<XmlElemSerializerBase*> XmlSubElementListType;
		XmlSubElementListType mSubElemList;

	public:
		XmlSubElementListType& GetList();
	};

	///////////////////////////////////////////////////////////////////////////////

	struct xmlSavingTreeNode
	{
		// 非0，表示需要写入一个结束标记
		// 否则，表示一个待处理的节点
		bool isEnd;

		XmlElemSerializerBase *node;
	};

	class NOTSTD_API XmlElemSerializer
	{
		std::string mPath;

	protected:
		bool WriteElemPropList(XmlElemSerializerBase *elem, xmlTextWriterPtr tWriter);
		bool WriteElemStart(XmlElemSerializerBase *elem, xmlTextWriterPtr tWriter);
		bool WriteElemEnd(XmlElemSerializerBase *elem, xmlTextWriterPtr tWriter);
		void AddSubNodeHelper(List<xmlSavingTreeNode> &tnList, XmlElemSerializerBase *elem);

	public:
		XmlElemSerializer(const std::string &path);

		bool Store(XmlElemSerializerBase *root);
		void Load(XmlElemSerializerBase *root) throw();

		static void TravElemTreeByLevel(XmlElemSerializerBase *root,
			std::function<void(notstd::SubElementList*)> funcSubList,
			std::function<bool(XmlElemSerializerBase*)> func,
			std::function<void(XmlPropSerializerBase*, const XmlFlagItem *)> funcProp);
		static void DFSTheElemTree(XmlElemSerializerBase *root,
			std::function<void(notstd::SubElementList*)> funcSubList,
			std::function<bool(XmlElemSerializerBase*)> func,
			std::function<void(XmlPropSerializerBase*, const XmlFlagItem *)> funcProp);
		static void InitializeDOMTree(XmlElemSerializerBase *base);

		static void ReleaseXmlDomTree(XmlElemSerializerBase *root);
	};

	///////////////////////////////////////////////////////////////////////////////

#pragma warning(pop)
}
