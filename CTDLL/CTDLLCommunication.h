#pragma once
#include "CTDLLService.h"
#include "CTDLL.h"

typedef struct Application Application;
typedef struct TParam_LARequest TParam_LARequest;
typedef struct TParam_ListPassengers TParam_ListPassengers;
typedef struct TParam_NotificationReceiver_NP TParam_NotificationReceiver_NP;

struct TParam_LARequest{
	Application* app;
	LARequest request;
};

struct TParam_ListPassengers{
	Application* app;
};

struct TParam_NotificationReceiver_NP{
	Application* app;
};

CTDLL_API DWORD WINAPI Thread_SendLARequests(LPVOID _param);
CTDLL_API DWORD WINAPI Thread_NotificationReceiver_NP(LPVOID _param);
