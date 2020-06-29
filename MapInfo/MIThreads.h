#pragma once
#include "MIService.h"

typedef struct Application Application;

typedef struct TParam_RefreshRoutine TParam_RefreshRoutine;

struct TParam_RefreshRoutine{
	HANDLE hWnd;
};

DWORD WINAPI Thread_RefreshRoutine(LPVOID _param);
