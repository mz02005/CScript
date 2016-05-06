#include "stdafx.h"
#include "serialCompletionPort.h"
#include "notstd/stringHelper.h"

Serial::Serial(IOCompletionManager *ioManager)
	: mSerial(INVALID_HANDLE_VALUE)
	, mCompletionManager(ioManager)
{
}

Serial::~Serial()
{
	Close();
}

bool Serial::PostRead(IOCompleteData *completeData)
{
	return false;

	//BOOL r = ::ReadFile(mSerial, completeData->mBuffer->GetBuffer(),
	//	completeData->mBuffer->GetCap(), NULL, completeData);
	//return (r || GetLastError() == ERROR_IO_PENDING);
}

bool Serial::PostWrite(IOCompleteData *completeData)
{
	return false;

	//BOOL r = ::WriteFile(mSerial, completeData->mBuffer->GetBuffer(),
	//	completeData->mBuffer->GetSize(), NULL, completeData);
	//return (r || GetLastError() == ERROR_IO_PENDING);
}

bool Serial::Read(char *buf, uint32_t &bufSize)
{
	DWORD readed;
	BOOL r = ::ReadFile(mSerial, buf, bufSize, &readed, NULL);
	if (!r)
		return false;
	bufSize = readed;
	return true;
}

bool Serial::Write(const char *buf, uint32_t &bufSize)
{
	DWORD written;
	BOOL r = ::WriteFile(mSerial, buf, bufSize, &written, NULL);
	if (!r)
		return false;
	bufSize = written;
	return true;
}

bool Serial::SetReadTimeout(DWORD timeout)
{
	COMMTIMEOUTS ct;
	if (!::GetCommTimeouts(mSerial, &ct))
		return false;
	ct.ReadTotalTimeoutConstant = timeout;
	return !!::SetCommTimeouts(mSerial, &ct);
}

bool Serial::Open(int commId, DWORD baudrate, 
	BYTE byteSize, BYTE stopBits, BYTE parity)
{
	bool r;
	BOOL bRet;
	DCB dcb;
	COMMTIMEOUTS ct;
	std::string strComm;

	r = false;
	Close();

	StringHelper::Format(strComm, "COM%d", commId);

	mSerial = ::CreateFileA(
		strComm.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0, //FILE_FLAG_OVERLAPPED,
		NULL);

	if (mSerial == INVALID_HANDLE_VALUE) {
		goto EndOpen;
	}

	dcb.DCBlength = sizeof(dcb);
	bRet = ::GetCommState(mSerial, &dcb);
	if (!bRet)
	{
		goto EndOpen;
	}

	dcb.fParity = TRUE;
	dcb.BaudRate = baudrate;
	dcb.ByteSize = byteSize;
	dcb.Parity = parity;
	dcb.StopBits = stopBits;

	bRet = ::SetCommState(mSerial, &dcb);
	if (!bRet) {
		goto EndOpen;
	}

	memset(&ct, 0, sizeof(ct));
	ct.ReadIntervalTimeout = MAXDWORD;
	ct.ReadTotalTimeoutMultiplier = 0;
	ct.ReadTotalTimeoutConstant = 50;
	ct.WriteTotalTimeoutConstant = 0;
	ct.WriteTotalTimeoutMultiplier = 20;
	bRet = ::SetCommTimeouts(mSerial, &ct);
	if (!bRet) {
		goto EndOpen;
	}

	//if (!mCompletionManager->BindIOHandle(mSerial))
	//	goto EndOpen;

	r = true;

EndOpen:
	if (!r)
		Close();
	return r;
}

void Serial::Close()
{
	if (mSerial != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(mSerial);
		mSerial = INVALID_HANDLE_VALUE;
	}
}

void Serial::OnCustumMsg(IOCompleteData *completeData)
{
}

void Serial::OnReceive(IOCompleteData *completeData)
{
}

void Serial::OnSend(IOCompleteData *completeData)
{
}

void Serial::OnConnect(bool connectOK)
{
}

void Serial::OnAccept(const NetAddress &client, const NetAddress &serv)
{
}

void Serial::OnClose()
{
}

bool Serial::IsError()
{
	return false;
}

bool Serial::SendPartial(IOCompleteData *completeData)
{
	return true;
}
