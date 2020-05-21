#pragma once
#include "CenDLL.h"

//Requests
typedef struct QnARequest QnARequest;
typedef struct LoginRequest LoginRequest;
typedef struct NTInterestRequest NTInterestRequest;

//Responses
typedef struct LoginResponse LoginResponse;
typedef enum LoginResponseType LoginResponseType;
typedef enum NTInterestResponse NTInterestResponse;

//Other Enums
typedef enum RequestType RequestType;

//SHM
typedef struct NewTransportBuffer NewTransportBuffer;

struct LoginRequest{
	TCHAR licensePlate[STRING_SMALL];
	float coordX;
	float coordY;
};

struct NTInterestRequest{
	TCHAR licensePlate[STRING_SMALL];
	TCHAR idPassenger[STRING_SMALL];
};

struct NewTransportBuffer{
	Passenger transportRequests[NTBUFFER_MAX];
	int head;
};

struct LoginResponse{
	LoginResponseType loginResponseType;
	int mapWidth;
	int mapHeight;
};

enum LoginResponseType{
	LR_SUCCESS,
	LR_INVALID_UNDEFINED,
	LR_INVALID_CLOSED,
	LR_INVALID_FULL,
	LR_INVALID_POSITION,
	LR_INVALID_EXISTS
};

enum NTInterestResponse{
	NTIR_SUCCESS,
	NTIR_INVALID_UNDEFINED,
	NTIR_INVALID_ID,
	NTIR_INVALID_CLOSED
};

struct QnARequest{
	union{
		LoginRequest loginRequest;
		NTInterestRequest ntIntRequest;
	};
	RequestType requestType;
	union{
		LoginResponse loginResponse;
		NTInterestResponse ntIntResponse;
	};
};

enum RequestType{ //Types of requests 
	RT_LOGIN, //Taxi login
	RT_NT_INTEREST //Taxi interest in a transport request
};