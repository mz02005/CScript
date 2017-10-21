#include "stdafx.h"
#include "hmhttptest.h"

IMPLEMENT_XMLSERIAL_ELEM(add, XmlElemSerializerBase)

BEGIN_XMLSERIAL_FLAG_TABLE(add, XmlElemSerializerBase)
	XMLSERIAL_PROP_ENTRY("local", mLocal)
	XMLSERIAL_PROP_ENTRY("url", mUrl)
	XMLSERIAL_PROP_ENTRY("username", mUsername)
	XMLSERIAL_PROP_ENTRY("password", mPassword)
	XMLSERIAL_PROP_ENTRY("extwhitelist", mExtWhiteList)
	XMLSERIAL_PROP_ENTRY("extblacklist", mExtBlackList)
END_XMLSERIAL_FLAG_TABLE()

void add::Normalize(XmlElemSerializerBase *parent)
{
	mRealExtWhiteList = notstd::StringHelper::SplitString(mExtWhiteList.mVal, ",");
	mRealExtBlackList = notstd::StringHelper::SplitString(mExtBlackList.mVal, ",");
}

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_XMLSERIAL_ELEM(releaseList, XmlElemSerializerBase)

BEGIN_XMLSERIAL_FLAG_TABLE(releaseList, XmlElemSerializerBase)
	XMLSERIAL_SUBLIST_ENTRY("add", mAddList)
END_XMLSERIAL_FLAG_TABLE()

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_XMLSERIAL_ELEM(basicauth, XmlElemSerializerBase)

BEGIN_XMLSERIAL_FLAG_TABLE(basicauth, XmlElemSerializerBase)
	XMLSERIAL_PROP_ENTRY("username", mUsername)
	XMLSERIAL_PROP_ENTRY("password", mPassword)
END_XMLSERIAL_FLAG_TABLE()

basicauth::basicauth()
{
	mUsername.mVal = "guest";
	mPassword.mVal = "iamabadbear";
}

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_XMLSERIAL_ELEM(network, XmlElemSerializerBase)

BEGIN_XMLSERIAL_FLAG_TABLE(network, XmlElemSerializerBase)
	XMLSERIAL_PROP_ENTRY("sendBufSize", mSendBufSize)
END_XMLSERIAL_FLAG_TABLE()

network::network()
{
	mSendBufSize.mVal = 16384;
}

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_XMLSERIAL_ELEM(filesystem, XmlElemSerializerBase)

BEGIN_XMLSERIAL_FLAG_TABLE(filesystem, XmlElemSerializerBase)
	XMLSERIAL_ELEM_ENTRY("basicauth", mBasicAuth)
	XMLSERIAL_ELEM_ENTRY("network", mNetwork)
END_XMLSERIAL_FLAG_TABLE()

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_XMLSERIAL_ELEM(httptest,XmlElemSerializerBase)

BEGIN_XMLSERIAL_FLAG_TABLE(httptest, XmlElemSerializerBase)
	XMLSERIAL_ELEM_ENTRY("filesystem",mFileSystem)
	XMLSERIAL_ELEM_ENTRY("releaseList",mReleaseList)
END_XMLSERIAL_FLAG_TABLE()
