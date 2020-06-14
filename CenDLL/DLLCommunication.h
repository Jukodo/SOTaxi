#pragma once
#include "CenDLL.h"

//Requests
typedef struct QnARequest QnARequest;
typedef struct TaxiLoginRequest TaxiLoginRequest;
typedef struct NTInterestRequest NTInterestRequest;

typedef struct TossRequest TossRequest;
typedef struct TossPosition TossPosition;
typedef struct TossState TossState;
typedef struct TossLogout TossLogout;

//Responses
typedef struct TaxiLoginResponse TaxiLoginResponse;
typedef enum TaxiLoginResponseType TaxiLoginResponseType;
typedef enum NTInterestResponse NTInterestResponse;

//Other Enums
typedef enum QnARequestType QnARequestType;
typedef enum TossRequestType TossRequestType;
typedef enum CommsTCType CommsTCType;
typedef enum CommsPCType CommsPCType;
typedef enum ShutdownType ShutdownType;

//SHM
typedef struct NewTransportBuffer NewTransportBuffer;
typedef struct TossRequestsBuffer TossRequestsBuffer;

//Named Pipe
typedef struct CommsTC CommsTC;
typedef struct CommsTC_Identity CommsTC_Identity;
typedef struct CommsTC_Assign CommsTC_Assign;
typedef struct CommsTC_Shutdown CommsTC_Shutdown;
typedef struct CommsPC CommsPC;

struct TaxiLoginRequest{
	TCHAR licensePlate[STRING_SMALL];
	double coordX;
	double coordY;
};

struct NTInterestRequest{
	TCHAR licensePlate[STRING_SMALL];
	TCHAR idPassenger[STRING_SMALL];
};

struct TaxiLoginResponse{
	TaxiLoginResponseType taxiLoginResponseType;
	int mapWidth;
	int mapHeight;
};

enum TaxiLoginResponseType{
	TLR_SUCCESS,
	TLR_INVALID_UNDEFINED,
	TLR_INVALID_CLOSED,
	TLR_INVALID_FULL,
	TLR_INVALID_POSITION,
	TLR_INVALID_EXISTS
};

enum NTInterestResponse{
	NTIR_SUCCESS,
	NTIR_INVALID_UNDEFINED,
	NTIR_INVALID_ID,
	NTIR_INVALID_CLOSED
};

struct QnARequest{
	union{
		TaxiLoginRequest taxiLoginRequest;
		NTInterestRequest ntIntRequest;
	};
	QnARequestType requestType;
	union{
		TaxiLoginResponse taxiLoginResponse;
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

struct TossLogout{
	TCHAR licensePlate[STRING_LICENSEPLATE];
};

struct TossRequest{
	union{
		TossPosition tossPosition;
		TossState tossState;
		TossLogout tossLogout;
	};
	TossRequestType tossType;
};

enum TossRequestType{ //Types of Toss requests 
	TRT_TAXI_POSITION, //Taxi new position
	TRT_TAXI_STATE, //Taxi new state
	TRT_TAXI_LOGOUT //Taxi logout
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

struct CommsTC_Identity{
	TCHAR licensePlate[STRING_LICENSEPLATE];
};

struct CommsTC_Assign{
	TCHAR passId[STRING_SMALL];
	double coordX;
	double coordY;
};

struct CommsTC_Shutdown{
	ShutdownType shutdownType;
	TCHAR message[STRING_LARGE];
};

enum CommsTCType{
	CTC_ASSIGNED,
	CTC_SHUTDOWN
};

struct CommsTC{
	union{
		CommsTC_Assign assignComm;
		CommsTC_Shutdown shutdownComm;
	};
	CommsTCType commType;
};

enum CommsPCType{
	PTC_LOGIN,
	PTC_LISTPASS,
	PTC_DISCONNECT
};

struct CommsPC{
	
	CommsPCType commType;
};

enum Shutdown_Type{
	ST_GLOBAL,
	ST_KICKED
};