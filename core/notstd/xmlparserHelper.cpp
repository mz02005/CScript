#include "stdafx.h"
#include "xmlparserHelper.h"
#include "simpleTool.h"

namespace notstd {

	void xmlSAXParser::OnStartElement(const char *name, const char **atts)
	{
	}

	void xmlSAXParser::OnEndElement(const char *name)
	{
	}

	void xmlSAXParser::OnCharacters(const char *ch, int len)
	{
	}

	void xmlSAXParser::BeforeParse(xmlSAXHandler *handler)
	{
		handler->startElement = &xmlSAXParser::onStartElement;
		handler->endElement = &xmlSAXParser::onEndElement;
		handler->characters = &xmlSAXParser::onCharacters;
	}

	void xmlSAXParser::AfterParse()
	{
	}

	void xmlSAXParser::onStartElement(void *ctx, const xmlChar *name, const xmlChar **atts)
	{
		xmlParserCtxt *context = reinterpret_cast<xmlParserCtxt*>(ctx);
		xmlSAXParser *parser = reinterpret_cast<xmlSAXParser*>(context->_private);
		parser->mNameQueue->push_back(std::string(reinterpret_cast<const char*>(name)));
		parser->OnStartElement(reinterpret_cast<const char*>(name),
			reinterpret_cast<const char**>(atts));
	}

	void xmlSAXParser::onEndElement(void *ctx, const xmlChar *name)
	{
		xmlParserCtxt *context = reinterpret_cast<xmlParserCtxt*>(ctx);
		xmlSAXParser *parser = reinterpret_cast<xmlSAXParser*>(context->_private);
		parser->OnEndElement(reinterpret_cast<const char*>(name));
		parser->mNameQueue->pop_back();
	}

	void xmlSAXParser::onCharacters(void *ctx, const xmlChar *ch, int len)
	{
		xmlParserCtxt *context = reinterpret_cast<xmlParserCtxt*>(ctx);
		xmlSAXParser *parser = reinterpret_cast<xmlSAXParser*>(context->_private);
		parser->OnCharacters(reinterpret_cast<const char*>(ch), len);
	}

	xmlSAXParser::xmlSAXParser()
		: mNameQueue(new std::list<std::string>)
	{
		xmlKeepBlanksDefault(0);
	}

	xmlSAXParser::~xmlSAXParser()
	{
		delete mNameQueue;
	}

	xmlDocPtr xmlSAXParser::ParseFile(const std::string &pathName)
	{
		xmlSAXHandler handler = { 0 };
		BeforeParse(&handler);
		xmlDocPtr doc = xmlSAXParseFileWithData(&handler, pathName.c_str(), 1, this);
		AfterParse();
		return doc;
	}

	xmlDocPtr xmlSAXParser::ParseBuffer(const char *buffer, std::size_t bufferSize)
	{
		xmlSAXHandler handler = { 0 };
		BeforeParse(&handler);
		xmlDocPtr doc = xmlSAXParseMemoryWithData(&handler, buffer,
			static_cast<int>(bufferSize), 1, this);
		AfterParse();
		return doc;
	}

	///////////////////////////////////////////////////////////////////////////////

	xmlPath::xmlPath()
		: mContext(NULL)
		, mPathObject(NULL)
	{
	}

	xmlPath::~xmlPath()
	{
		Close();
	}

	bool xmlPath::Find(xmlDocPtr doc, const char *strPath)
	{
		Close();
		mContext = xmlXPathNewContext(doc);
		if (!mContext)
			return false;
		mPathObject = xmlXPathEvalExpression(
			reinterpret_cast<const xmlChar*>(strPath), mContext);
		if (!mPathObject || xmlXPathNodeSetIsEmpty(mPathObject->nodesetval))
			return false;
		return true;
	}

	void xmlPath::Close()
	{
		if (mPathObject) {
			xmlXPathFreeObject(mPathObject);
			mPathObject = NULL;
		}
		if (mContext) {
			xmlXPathFreeContext(mContext);
			mContext = NULL;
		}
	}

	///////////////////////////////////////////////////////////////////////////////

	xmlString::xmlString()
		: mString(NULL)
	{
	}

	xmlString::xmlString(xmlChar *str)
		: mString(str)
	{
	}

	xmlString::~xmlString()
	{
		if (mString) {
			xmlFree(mString);
			mString = NULL;
		}
	}

	///////////////////////////////////////////////////////////////////////////////

	xmlDocumentHelper::xmlDocumentHelper()
		: mDoc(NULL)
	{
		xmlKeepBlanksDefault(0);
	}

	xmlDocumentHelper::~xmlDocumentHelper()
	{
		ReleaseDoc();
	}

	bool xmlDocumentHelper::LoadFromBuffer(const char *buffer, std::size_t size)
	{
		ReleaseDoc();
		mDoc = xmlParseMemory(buffer, static_cast<int>(size));
		return (mDoc != NULL);
	}

	bool xmlDocumentHelper::LoadFromFile(const char *filePathName)
	{
		ReleaseDoc();
		mDoc = xmlParseFile(filePathName);
		return (mDoc != NULL);
	}

	void xmlDocumentHelper::ReleaseDoc()
	{
		if (mDoc) {
			xmlFreeDoc(mDoc);
			mDoc = NULL;
		}
	}

	void xmlDocumentHelper::DestroyDocFromFile(xmlDocumentHelper *doc)
	{
		delete doc;
	}

	xmlDocumentHelper* xmlDocumentHelper::CreateDocFromFile(const char *filePathName)
	{
		Handle<STDCFILEHandle> file = ::fopen(filePathName, "rb");
		if (!file)
			return NULL;

		fseek(file, 0, SEEK_END);
		std::size_t len = std::size_t(ftell(file));
		if (!len)
			return NULL;

		xmlDocumentHelper *r = new xmlDocumentHelper;
		std::string buf;
		buf.resize(len);
		fseek(file, 0, SEEK_SET);
		if (fread(&buf[0], 1, len, file) != len)
		{
			delete r;
			return nullptr;
		}
		if (!r->LoadFromBuffer(&buf[0], len))
		{
			delete r;
			return NULL;
		}
		return r;
	}
}
