#pragma once
#include "MIThreads.h"

DWORD WINAPI Thread_RefreshRoutine(LPVOID _param){
	TParam_RefreshRoutine* param = (TParam_RefreshRoutine*) _param;

    HANDLE hWTRefreshRate = CreateWaitableTimer(
        NULL,	                    //Security Attributes
        FALSE,	                    //Manual Reset
        NAME_WTIMER_REFRESH_RATE);	//WaitableTimer name
    if(GetLastError() == ERROR_ALREADY_EXISTS)
        hWTRefreshRate = OpenWaitableTimer(TIMER_ALL_ACCESS, FALSE, NAME_WTIMER_REFRESH_RATE);
    Utils_DLL_Register(NAME_WTIMER_REFRESH_RATE, DLL_TYPE_WAITABLETIMER);

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