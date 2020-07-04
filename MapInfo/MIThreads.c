#pragma once
#include "MIThreads.h"

DWORD WINAPI Thread_RefreshRoutine(LPVOID _param){
	TParam_RefreshRoutine* param = (TParam_RefreshRoutine*) _param;

    HANDLE hWTRefreshRate = CreateWaitableTimer(
        NULL,	//Security Attributes
        FALSE,	//Manual Reset
        NULL);	//WaitableTimer name
    Utils_DLL_Register(TEXT("unnamedWT:miThreads.c:line4"), DLL_TYPE_WAITABLETIMER);

    if(hWTRefreshRate == NULL){
        return -501;
    }

    LARGE_INTEGER liTime;
    liTime.QuadPart = -10000000LL * REFRESH_RATE;

    SetWaitableTimer(
        hWTRefreshRate,
        &liTime,
        1000 * REFRESH_RATE,
        NULL,
        NULL,
        FALSE
    );

    RECT windowSize;
    GetClientRect(param->hWnd, &windowSize);

    while(WaitForSingleObject(hWTRefreshRate, INFINITE) != WAIT_ABANDONED_0){
        InvalidateRect(param->hWnd, &windowSize, FALSE);
    }

	free(param);
	return 501;
}