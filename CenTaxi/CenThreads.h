#pragma once
#include "CenService.h"

typedef struct Application Application;

typedef struct TParam_QnARequest TParam_QnARequest;
typedef struct TParam_TaxiAssignment TParam_TaxiAssignment;
typedef struct TParam_ConsumeTossRequests TParam_ConsumeTossRequests;
typedef struct TParam_ConnectingTaxiPipes TParam_ConnectingTaxiPipes;
typedef struct TParam_ReadConPassNPQnA TParam_ReadConPassNPQnA;
typedef struct TParam_ReadConPassNPToss TParam_ReadConPassNPToss;

struct TParam_QnARequest{
	Application* app;
};

struct TParam_TaxiAssignment{
	Application* app;
	int myIndex; //Each passenger has a thread for this matter and each thread will know the index of the respective passenger
};

struct TParam_ConsumeTossRequests{
	Application* app;
};

struct TParam_ConnectingTaxiPipes{
	Application* app;
};

struct TParam_ReadConPassNPQnA{
	Application* app;
};

struct TParam_ReadConPassNPToss{
	Application* app;
};

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID _param);
DWORD WINAPI Thread_TaxiAssignment(LPVOID _param);
DWORD WINAPI Thread_ConsumeTossRequests(LPVOID _param);
DWORD WINAPI Thread_ConnectingTaxiPipes(LPVOID _param);
DWORD WINAPI Thread_ReadConPassNPQnA(LPVOID _param);
DWORD WINAPI Thread_ReadConPassNPToss(LPVOID _param);