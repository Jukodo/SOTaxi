#pragma once
#include "CPService.h"

typedef struct Application Application;

typedef struct TParam_NotificationReceiver_NamedPipe TParam_NotificationReceiver_NamedPipe;
typedef struct TParam_SendCommQnA TParam_SendCommQnA;
typedef struct TParam_SendCommToss TParam_SendCommToss;

struct TParam_NotificationReceiver_NamedPipe{
	Application* app;
};

struct TParam_SendCommQnA{
	Application* app;
	CommsP2C commPC;
};

struct TParam_SendCommToss{
	Application* app;
	CommsP2C commPC;
};

DWORD WINAPI Thread_NotificationReceiver_NamedPipe(LPVOID _param);
DWORD WINAPI Thread_SendCommQnA(LPVOID _param);
DWORD WINAPI Thread_SendCommToss(LPVOID _param);