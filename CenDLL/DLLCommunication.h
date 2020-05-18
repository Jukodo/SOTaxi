#pragma once
#include "CenDLL.h"

#define CMD_HELP TEXT("/help")
#define CMD_SPEED_UP TEXT("/speedUp")
#define CMD_SPEED_DOWN TEXT("/speedDown")
#define CMD_AUTORESP_ON TEXT("/autoRespOn")
#define CMD_AUTORESP_OFF TEXT("/autoRespOff")
#define CMD_DEFINE_CDN TEXT("/defineCDN")
#define CMD_REQUEST_INTEREST TEXT("/requestInterest")
#define CMD_CLOSEAPP TEXT("/closeApp")

//Requests
typedef struct QnARequest QnARequest;
typedef struct LoginRequest LoginRequest;
typedef struct NTInterestRequest NTInterestRequest;

//Responses
typedef enum LoginResponse LoginResponse;
typedef enum NTInterestResponse NTInterestResponse;

//Other Enums
typedef enum RequestType RequestType;
typedef enum TaxiCommands TaxiCommands;

//SHM
typedef struct NewTransportBuffer NewTransportBuffer;

struct LoginRequest{
	TCHAR licensePlate[STRING_SMALL];
	float coordX;
	float coordY;
};

struct NTInterestRequest{
	TCHAR idPassenger[STRING_SMALL];
};

struct NewTransportBuffer{
	Passenger transportRequests[NTBUFFER_MAX];
	int head;
};

enum LoginResponse{
	LR_SUCCESS,
	LR_INVALID_UNDEFINED,
	LR_INVALID_FULL,
	LR_INVALID_POSITION
};

enum NTInterestResponse{
	AR_SUCCESS,
	AR_INVALID_UNDEFINED
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

enum TaxiCommands{
	TC_HELP,
	TC_SPEED_UP,
	TC_SPEED_DOWN,
	TC_AUTORESP_ON,
	TC_AUTORESP_OFF,
	TC_DEFINE_CDN,
	TC_REQUEST_INTEREST,
	TC_CLOSEAPP,
	TC_UNDEFINED
};