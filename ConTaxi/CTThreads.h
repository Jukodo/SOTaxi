#pragma once
#include "CTService.h"

typedef struct Application Application;

typedef struct TParam_StepRoutine TParam_StepRoutine;
typedef struct TParam_DestinationChanger TParam_DestinationChanger;

struct TParam_StepRoutine{
	Application* app;
};

struct TParam_DestinationChanger{
	Application* app;
};

DWORD WINAPI Thread_StepRoutine(LPVOID _param);
DWORD WINAPI Thread_DestinationChanger(LPVOID _param);