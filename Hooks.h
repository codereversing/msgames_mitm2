#pragma once

#include <Windows.h>

void WINAPI SendDataHook(void *pThisPtr, char *pBuffer, unsigned int uiLength, int uk2, int uk3);
void WINAPI DecryptSocketDataHook(char *pBuffer, unsigned int uiLength);