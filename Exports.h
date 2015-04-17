#pragma once

#include <Windows.h>

typedef int (WINAPI *pBeginConnect)(void *pThisPtr, unsigned char *Unknown1, unsigned long Unknown2);
typedef int (WINAPI *pSendData)(void *pThisPtr, char *pBuffer, unsigned int uiLength, int uk2, int uk3);
typedef void (WINAPI *pDecryptSocketData)(void *pThisPtr);
typedef void (WINAPI *pDisconnect)(void *pThisPtr);

extern pBeginConnect BeginConnectFnc;
extern pSendData SendDataFnc;
extern pDecryptSocketData DecryptSocketDataFnc;
extern pDisconnect DisconnectFnc;

extern void *pThisPtr;

const bool GetFunctions(void);