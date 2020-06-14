#pragma once
#include "CPService.h"

typedef struct Application Application;

typedef struct TParam_ReadComms TParam_ReadComms;
typedef struct TParam_SendComm TParam_SendComm;

struct TParam_ReadComms{
	Application* app;
};

struct TParam_SendComm{
	Application* app;
	CommsP2C commPC;
};

DWORD WINAPI Thread_ReadComms(LPVOID _param);
DWORD WINAPI Thread_SendComm(LPVOID _param);