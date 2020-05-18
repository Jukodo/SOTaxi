#pragma once
#include "CenDLL.h"

#define CMD_HELP TEXT("/help")
#define CMD_SPEED_UP TEXT("/speedUp")
#define CMD_SPEED_DOWN TEXT("/speedDown")
#define CMD_AUTORESP_ON TEXT("/autoRespOn")
#define CMD_AUTORESP_OFF TEXT("/autoRespOff")
#define CMD_DEFINE_CDN TEXT("/defineCDN")
#define CMD_REQUEST_PASS TEXT("/requestPass")
#define CMD_CLOSEAPP TEXT("/closeApp")

//Requests
typedef struct QnARequest QnARequest;
typedef struct NewTransportBuffer NewTransportBuffer;
typedef struct LoginRequest LoginRequest;
typedef struct AssignRequest AssignRequest;
typedef struct CDNRequest CDNRequest;
typedef enum CommandRequest CommandRequest;
typedef enum VarRequest VarRequest;

//Responses
typedef enum LoginResponse LoginResponse;
typedef enum AssignResponse AssignResponse;
typedef enum CDNResponse CDNResponse;
typedef struct VarResponse VarResponse;
typedef enum CommandResponse CommandResponse;

//Other Enums
typedef enum RequestType RequestType;
typedef enum TaxiCommands TaxiCommands;

struct LoginRequest{
	TCHAR licensePlate[STRING_SMALL];
	float coordX;
	float coordY;
};

struct AssignRequest{
	TCHAR idPassenger[STRING_SMALL];
};

struct CDNRequest{
	int value;
};

struct NewTransportBuffer{
	Passenger transportRequests[NTBUFFER_MAX];
	int head;
};

enum CommandRequest{
	CR_LISTPASS,
	CR_SPEED_UP,
	CR_SPEED_DOWN,
	CR_AUTORESP_ON,
	CR_AUTORESP_OFF,
	CR_CLOSEAPP
};

enum VarRequest{
	VR_MAX_PASSENGERS,
	VR_MAX_TAXIS
};

enum LoginResponse{
	LR_SUCCESS,
	LR_INVALID_UNDEFINED,
	LR_INVALID_FULL,
	LR_INVALID_POSITION
};

enum AssignResponse{
	AR_SUCCESS,
	AR_INVALID_UNDEFINED
};

enum CDNResponse{
	CDN_SUCCESS,
	CDN_INVALID_UNDEFINED
};

enum CommandResponse{
	CR_SUCCESS,
	CR_INVALID_UNDEFINED
};

struct VarResponse{
	int maxTaxis;
	int maxPassengers;
};

struct QnARequest{
	union{
		LoginRequest loginRequest;
		AssignRequest assignRequest;
		CDNRequest cdnRequest;
		CommandRequest cmdRequest;
		VarRequest varRequest;
	};
	RequestType requestType;
	union{
		LoginResponse loginResponse;
		AssignResponse assignResponse;
		CDNResponse cdnResponse;
		CommandResponse cmdResponse;
		VarResponse varResponse;
	};
};

enum RequestType{ //Types of requests 
	RT_LOGIN, //Taxi login
	RT_ASSIGN, //Taxi request to be assigned to a new passenger
	RT_CDN, //Taxi request to define a new number for CDN
	RT_VAR //Ask for a var (maxTaxi, maxPassenger)
};

enum TaxiCommands{
	TC_HELP,
	TC_SPEED_UP,
	TC_SPEED_DOWN,
	TC_AUTORESP_ON,
	TC_AUTORESP_OFF,
	TC_DEFINE_CDN,
	TC_REQUEST_PASS,
	TC_CLOSEAPP,
	TC_UNDEFINED
};