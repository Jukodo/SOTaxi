#pragma once
#include "CTDLLService.h"
#include "CTDLL.h"

typedef struct Application Application;
typedef struct TParam_QnARequest TParam_QnARequest;
typedef struct TParam_NotificationReceiver_NT TParam_NotificationReceiver_NT;
typedef struct TParam_TossRequest TParam_TossRequest;

struct TParam_QnARequest{
	Application* app;
	QnARequest request;
};

struct TParam_NotificationReceiver_NT{
	Application* app;
};

struct TParam_TossRequest{
	Application* app;
	TossRequest tossRequest;
};

CTDLL_API DWORD WINAPI Thread_SendQnARequests(LPVOID _param);
CTDLL_API DWORD WINAPI Thread_NotificationReceiver_NewTransport(LPVOID _param);
CTDLL_API DWORD WINAPI Thread_TossRequest(LPVOID _param);
