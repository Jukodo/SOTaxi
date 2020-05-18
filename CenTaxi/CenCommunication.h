#pragma once
#include "CenService.h"

typedef struct TParam_LARequest TParam_LARequest;

struct TParam_LARequest{
	Application* app;
};

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID request);