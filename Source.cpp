#define _CRT_SECURE_NO_WARNINGS //Ignore warning about freopen

#include <cstdio>
#include <Windows.h>

#include "Exports.h"
#include "Hooks.h"

#include "resource.h"

PVOID pExceptionHandler = NULL;

pBeginConnect BeginConnectFnc = NULL;
pSendData SendDataFnc = NULL;
pDecryptSocketData DecryptSocketDataFnc = NULL;
pDisconnect DisconnectFnc = NULL;

void *pThisPtr = NULL;

const bool AddBreakpoint(void *pAddress)
{
    SIZE_T dwSuccess = 0;

    MEMORY_BASIC_INFORMATION memInfo = { 0 };
    dwSuccess = VirtualQuery(pAddress, &memInfo, sizeof(MEMORY_BASIC_INFORMATION));
    if(dwSuccess == 0)
    {
        printf("VirtualQuery failed on %016X. Last error = %X\n", pAddress, GetLastError());
        return false;
    }

    DWORD dwOldProtections = 0;
    dwSuccess = VirtualProtect(pAddress, sizeof(DWORD_PTR), memInfo.Protect | PAGE_GUARD, &dwOldProtections);
    if(dwSuccess == 0)
    {
        printf("VirtualProtect failed on %016X. Last error = %X\n", pAddress, GetLastError());
        return false;
    }

    return true;
}

const bool SetBreakpoints(void)
{
    bool bRet = AddBreakpoint(BeginConnectFnc);
    bRet &= AddBreakpoint(SendDataFnc);
    bRet &= AddBreakpoint(DecryptSocketDataFnc);
    bRet &= AddBreakpoint(DisconnectFnc);

    return bRet;
}

LONG CALLBACK VectoredHandler(EXCEPTION_POINTERS *pExceptionInfo)
{
    if(pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION)
    {        
        pExceptionInfo->ContextRecord->EFlags |= 0x100;

        DWORD_PTR dwExceptionAddress = (DWORD_PTR)pExceptionInfo->ExceptionRecord->ExceptionAddress;
        CONTEXT *pContext = pExceptionInfo->ContextRecord;

        if(dwExceptionAddress == (DWORD_PTR)BeginConnectFnc)
        {
            pThisPtr = (void *)pContext->Rcx;
            printf("Starting connection. CStadiumSocket instance is at: %016X\n", pThisPtr);
        }
        else if(dwExceptionAddress == (DWORD_PTR)SendDataFnc)
        {
            DWORD_PTR *pdwParametersBase = (DWORD_PTR *)(pContext->Rsp + 0x28);
            SendDataHook((void *)pContext->Rcx, (char *)pContext->Rdx, (unsigned int)pContext->R8, (int)pContext->R9, (int)(*(pdwParametersBase)));
        }
        else if(dwExceptionAddress == (DWORD_PTR)DecryptSocketDataFnc + 0x1C3)
        {
            DecryptSocketDataHook((char *)pContext->Rdx, (unsigned int)pContext->R8);
        }
        else if(dwExceptionAddress == (DWORD_PTR)DisconnectFnc)
        {
            printf("Closing connection. CStadiumSocket instance is being set to NULL\n");
            pThisPtr = NULL;
        }

        return EXCEPTION_CONTINUE_EXECUTION;
    }
    
    if(pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
    {
        (void)SetBreakpoints();
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case ID_SEND:
            {
                //Possible condition here where Disconnect is called while custom chat message is being sent.
                if(pThisPtr != NULL)
                {
                    char strSendBuffer[512] = { 0 };
                    char strBuffer[256] = { 0 };
                    GetDlgItemTextA(hwndDlg, IDC_CHATTEXT, strBuffer, sizeof(strBuffer) - 1);

					//Extremely unsafe example code, careful...
					for (unsigned int i = 0; i < strlen(strBuffer); ++i)
					{
						if (strBuffer[i] == ' ')
						{
							memmove(&strBuffer[i + 3], &strBuffer[i + 1], strlen(&strBuffer[i]));
							strBuffer[i] = '%';
							strBuffer[i + 1] = '2';
							strBuffer[i + 2] = '0';
						}
					}

                    _snprintf(strSendBuffer, sizeof(strSendBuffer) - 1,
                        "CALL Chat sChatText=%s&sFontFace=MS%%20Shell%%20Dlg&arfFontFlags=0&eFontColor=12345&eFontCharSet=1\r\n",
                        strBuffer);

                    SendDataFnc(pThisPtr, strSendBuffer, (unsigned int)strlen(strSendBuffer), 0, 1);
                }
            }
                break;
        }
    default:
        return FALSE;
    }
    return TRUE;
}

DWORD WINAPI DlgThread(LPVOID hModule)
{
    return (DWORD)DialogBox((HINSTANCE)hModule, MAKEINTRESOURCE(DLG_MAIN), NULL, DialogProc);
}

int APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
        switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        (void)DisableThreadLibraryCalls(hModule);
        if(AllocConsole())
        {
            freopen("CONOUT$", "w", stdout);
            SetConsoleTitle(L"Console");
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            printf("DLL loaded.\n");
        }
        if(GetFunctions())
        {
            pExceptionHandler = AddVectoredExceptionHandler(TRUE, VectoredHandler);
            if(SetBreakpoints())
            {
                if(CreateThread(NULL, 0, DlgThread, hModule, 0, NULL) == NULL)
                    printf("Could not create dialog thread. Last error = %X\n", GetLastError());
            }
            else
            {
                printf("Could not set initial breakpoints.\n");
            }
            printf("CStadiumSocket::BeginConnect: %016X\n"
                "CStadiumSocket::SendData: %016X\n"
                "CStadiumSocket::DecryptSocketData: %016X\n"
                "CStadiumSocket::Disconnect: %016X\n",
                BeginConnectFnc, SendDataFnc, DecryptSocketDataFnc, DisconnectFnc);
        }
        break;

    case DLL_PROCESS_DETACH:
        //Clean up here usually
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}