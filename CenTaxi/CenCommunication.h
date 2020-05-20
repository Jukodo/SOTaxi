#pragma once
#include "CenService.h"

typedef struct TParam_QnARequest TParam_QnARequest;
typedef struct TParam_TaxiAssignment TParam_TaxiAssignment;

struct TParam_QnARequest{
	Application* app;
};

struct TParam_TaxiAssignment{
	Application* app;
	int myIndex; //Each passenger has a thread for this matter and each thread will know the index of the respective passenger
};

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID _param);
DWORD WINAPI Thread_TaxiAssignment(LPVOID _param);