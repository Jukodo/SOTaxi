#pragma once
#include "CenDLL.h"

//Responses
typedef struct Response Response;

//Requests
typedef struct LoginRequest LoginRequest;
typedef struct AssignRequest AssignRequest;
typedef struct LARequest LARequest;

//Enums
typedef enum RequestType RequestType;
typedef enum LoginResponse LoginResponse;
typedef enum AssignResponse AssignResponse;

struct Response{
	union{
		LoginResponse loginResponse;
		AssignResponse assignResponse;
	};
};

struct LoginRequest{
	TCHAR licensePlate[STRING_SMALL];
	int coordX;
	int coordY;
};

struct AssignRequest{
	int idPassenger;
};

struct LARequest{
	union{
		LoginRequest loginRequest;
		AssignRequest assignRequest;
	};
	RequestType requestType;
	union{
		LoginResponse loginResponse;
		AssignResponse assignResponse;
	};
};

enum RequestType{ LOGIN, ASSIGN };
enum LoginResponse{ LR_SUCCESS, LR_INVALID_UNDEFINED, LR_INVALID_FULL, LR_INVALID_POSITION };
enum AssignResponse{ AR_SUCCESS, AR_INVALID_UNDEFINED };
