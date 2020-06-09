#pragma once
#include "CTService.h"

typedef struct Application Application;

typedef struct TParam_StepRoutine TParam_StepRoutine;

struct TParam_StepRoutine{
	Application* app;
};
DWORD WINAPI Thread_StepRoutine(LPVOID _param);