#pragma once
#include "CTService.h"

typedef struct TParam_LARequest TParam_LARequest;
typedef struct TParam_ListPassengers TParam_ListPassengers;

struct TParam_LARequest{
	Application* app;
	LARequest request;
};

struct TParam_ListPassengers{
	Application* app;
};

DWORD WINAPI Thread_SendLARequests(LPVOID _param);
DWORD WINAPI Thread_ListPassenger(LPVOID _param);
