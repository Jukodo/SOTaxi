#pragma once
#include "CenDLL.h"

//Requests
typedef struct QnARequest QnARequest;
typedef struct LoginRequest LoginRequest;
typedef struct NTInterestRequest NTInterestRequest;

typedef struct TossRequest TossRequest;
typedef struct TossPosition TossPosition;
typedef struct TossState TossState;

//Responses
typedef struct LoginResponse LoginResponse;
typedef enum LoginResponseType LoginResponseType;
typedef enum NTInterestResponse NTInterestResponse;

//Other Enums
typedef enum QnARequestType QnARequestType;
typedef enum TossRequestType TossRequestType;

//SHM
typedef struct NewTransportBuffer NewTransportBuffer;
typedef struct TossRequestsBuffer TossRequestsBuffer;

struct LoginRequest{
	TCHAR licensePlate[STRING_SMALL];
	float coordX;
	float coordY;
};

struct NTInterestRequest{
	TCHAR licensePlate[STRING_SMALL];
	TCHAR idPassenger[STRING_SMALL];
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
	QnARequestType requestType;
	union{
		LoginResponse loginResponse;
		NTInterestResponse ntIntResponse;
	};
};

enum QnARequestType{ //Types of QnA requests 
	QnART_LOGIN, //Taxi login
	QnART_NT_INTEREST //Taxi interest in a transport request
};

struct TossPosition{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	double newX;
	double newY;
};

struct TossState{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	TaxiState newState;
};

struct TossRequest{
	union{
		TossPosition tossPosition;
		TossState tossState;
	};
	TossRequestType tossType;
};

enum TossRequestType{ //Types of Toss requests 
	TRT_TAXI_POSITION, //Taxi new position
	TRT_TAXI_STATE //Taxi new state
};

struct NewTransportBuffer{
	Passenger transportRequests[NTBUFFER_MAX];
	int head;
};

struct TossRequestsBuffer{
	TossRequest tossRequests[TOSSBUFFER_MAX];
	int head;
	int tail;
};