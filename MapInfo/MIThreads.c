#pragma once
#include "MIThreads.h"

DWORD WINAPI Thread_RefreshRoutine(LPVOID _param){
    TCHAR message[100];
    swprintf_s(message, 100, TEXT("%s Thread_RefreshRoutine started"), Utils_NewLine());
    OutputDebugString(message);

	TParam_RefreshRoutine* param = (TParam_RefreshRoutine*) _param;

    HANDLE hWTRefreshRate = CreateWaitableTimer(
        NULL,	                    //Security Attributes
        FALSE,	                    //Manual Reset
        NAME_WTIMER_REFRESH_RATE);	//WaitableTimer name
    if(GetLastError() == ERROR_ALREADY_EXISTS)
        hWTRefreshRate = OpenWaitableTimer(TIMER_ALL_ACCESS, FALSE, NAME_WTIMER_REFRESH_RATE);
    Utils_DLL_Register(NAME_WTIMER_REFRESH_RATE, DLL_TYPE_WAITABLETIMER);

    if(hWTRefreshRate == NULL){
        swprintf_s(message, 100, TEXT("%sWT failed"), Utils_NewLine());
        OutputDebugString(message);
        return false;
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
        swprintf_s(message, 100, TEXT("%s Thread_RefreshRoutine cycle IT.. refreshing from (%d %d) to (%d %d)"), Utils_NewLine(),
            windowSize.left,
            windowSize.top,
            windowSize.right,
            windowSize.bottom);
        OutputDebugString(message);

        InvalidateRect(param->hWnd, &windowSize, FALSE);
    }

    swprintf_s(message, 100, TEXT("%s Thread_RefreshRoutine died"), Utils_NewLine());
    OutputDebugString(message);
	free(param);
	return 501;
}