#pragma once
#include "CTService.h"

typedef struct TParam_LARequest TParam_LARequest;

struct TParam_LARequest{
	Application* app;
	LARequest request;
};

DWORD WINAPI Thread_SendLARequests(LPVOID request);