#include "CenDLL.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    {
        HINSTANCE hLib = LoadLibrary(TEXT("../ImplicitDLL/SO2_TP_DLL_32.dll"));
        FreeLibrary(hLib);
    }
        break;
    }
    return TRUE;
}

