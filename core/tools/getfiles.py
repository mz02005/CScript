#!/usr/bin/evn python
#coding=utf8
# pylint: disable=invalid-name

import httplib
import base64
import os
import urllib

targetUrl = '192.168.111.128'
port = 10088;
# root = '/system/Users/haifengxie/buyuClient/';
root = '/system/home/mz/TeamTalk/'

find1 = '<th align=\"left\"><a href=\"'
find3 = '<th align=\"left\">'

localRoot = 'd:\\theSource\\'

def FindInPage(theData):
    result = []
    begin = 0

    while True:
        singleResult = []
        f1 = theData.find(find1, begin)
        if f1 < 0:
            return result
        f2 = theData.find('\"', f1 + len(find1))
        if f2 < 0:
            return result
        singleResult.append(theData[f1 + len(find1):f2])
        lenUrl = len(singleResult[0])
        f3 = theData.find(find3, f2 + 1)
        if f3 < 0:
            return result
        f4 = theData.find('<', f3 + len(find3))
        if f4 < 0:
            return result
        singleResult.append(theData[f3 + len(find3):f4])
        result.append(singleResult)
        begin = f4 + 1
    return result

def DownloadSinglePage(url):
    httpClient = None;
    auth = base64.b64encode('guest'+ ':'+ 'iamabadbear') 
    headers = {"Authorization": "Basic "+ auth}

    data = ''
    try:
        httpClient = httplib.HTTPConnection(targetUrl, port, timeout=10)
        httpClient.request('GET', url, headers=headers)
        response = httpClient.getresponse()
        print response.status
        #print response.reason
        data = response.read()
        return (response.status, data)
    except Exception, e:
        print e
    finally:
        if httpClient:
            httpClient.close()

#data = DownloadSinglePage(root)
#theResult = FindInPage(data[1])
#for sr in theResult:
#    print sr[1] + ': ' + sr[0]

def DownloadAndCreateLocalFile(urlRoot, localRoot, theUrl):
    fileData = DownloadSinglePage(theUrl)
    relativeUrl = theUrl[len(urlRoot):]
    print relativeUrl
    fullPath = localRoot + relativeUrl
    fullPath = fullPath.replace('/', '\\')
    fullPath = urllib.unquote(fullPath)
    print fullPath
    fullPath = fullPath.decode('utf-8')
    print fullPath
    try:
        os.makedirs(fullPath[0:fullPath.rfind('\\')])
    except OSError as exc:
        pass

    if fileData == None:
        return
    
    # Write file
    wf = open(fullPath, 'wb')
    if wf == None:
        pass
    try:
        wf.write(fileData[1])
    finally:
        wf.close()
        

def BeginFromRoot():
    rootData = DownloadSinglePage(root)
    totalResult = FindInPage(rootData[1])
    while len(totalResult) > 0:
        ff = totalResult[0]
        totalResult.pop(0)
        if ff[1] == 'FILE':
            DownloadAndCreateLocalFile(root, localRoot, ff[0])
        else:
            tempData = DownloadSinglePage(ff[0])
            totalResult.extend(FindInPage(tempData[1]))

if __name__ == "__main__":
    BeginFromRoot()