#pragma once
#include "Service.h"

typedef struct SendRequest{
	Application* app;
	union{
		LoginRequest loginRequest;
		AssignRequest assignRequest;
	};
	RequestType requestType;
}SendRequest;

DWORD WINAPI Thread_SendRequests(LPVOID request);