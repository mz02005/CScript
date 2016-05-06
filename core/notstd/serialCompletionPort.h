#pragma once
#include "ioCompletionManager.h"

class NOTSTD_API Serial : public IOOperationBase
{
private:
	HANDLE mSerial;
	IOCompletionManager *mCompletionManager;

protected:
	virtual void OnCustumMsg(IOCompleteData *completeData);
	virtual void OnReceive(IOCompleteData *completeData);
	virtual void OnSend(IOCompleteData *completeData);
	virtual void OnConnect(bool connectOK);
	virtual void OnAccept(const NetAddress &client, const NetAddress &serv);
	virtual void OnClose();
	virtual bool IsError();

	virtual bool SendPartial(IOCompleteData *completeData);

public:
	bool Open(int commId, DWORD baudrate, BYTE byteSize, BYTE stopBits, BYTE parity);
	void Close();

	virtual bool PostRead(IOCompleteData *completeData);
	virtual bool PostWrite(IOCompleteData *completeData);
	virtual bool Read(char *buf, uint32_t &bufSize);
	virtual bool Write(const char *buf, uint32_t &bufSize);

	bool SetReadTimeout(DWORD timeout);

	Serial(IOCompletionManager *ioManager);
	virtual ~Serial();
};
