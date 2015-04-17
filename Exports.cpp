#pragma comment(lib, "Dbghelp.lib")

#include "Exports.h"

#include <stdio.h>
#include <Dbghelp.h>

const bool GetFunctions(void)
{
    (void)SymSetOptions(SYMOPT_UNDNAME);
    if(SymInitialize(GetCurrentProcess(), "", TRUE))
    {
        IMAGEHLP_SYMBOL64 imageHelp = { 0 };
        imageHelp.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);

        (void)SymGetSymFromName64(GetCurrentProcess(), "CStadiumSocket::BeginConnect", &imageHelp);
        BeginConnectFnc = (pBeginConnect)imageHelp.Address;

        (void)SymGetSymFromName64(GetCurrentProcess(), "CStadiumSocket::SendData", &imageHelp);
        SendDataFnc = (pSendData)imageHelp.Address;

        (void)SymGetSymFromName64(GetCurrentProcess(), "CStadiumSocket::DecryptSocketData", &imageHelp);
        DecryptSocketDataFnc = (pDecryptSocketData)imageHelp.Address;  

        (void)SymGetSymFromName64(GetCurrentProcess(), "CStadiumSocket::Disconnect", &imageHelp);
        DisconnectFnc = (pDisconnect)imageHelp.Address;

    }
    else
    {
        printf("Could not initialize symbols. Last error = %X", GetLastError());
    }
    return ((BeginConnectFnc != NULL) && (SendDataFnc != NULL)
        && (DecryptSocketDataFnc != NULL) && (DisconnectFnc != NULL));
}