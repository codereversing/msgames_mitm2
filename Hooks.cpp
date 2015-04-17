#include "Hooks.h"

#include <stdio.h>

void WINAPI SendDataHook(void *pThisPtr, char *pBuffer, unsigned int uiLength, int uk2, int uk3)
{
    printf("--- Sending ---\n"
        "Input: %.*s\n",
        uiLength, pBuffer);
}

void WINAPI DecryptSocketDataHook(char *pBuffer, unsigned int uiLength)
{
    printf("--- Receiving ---\n"
        "Buffer: %.*s\n",
        uiLength, pBuffer);
}