#pragma once
#include "CenDLL.h"

//Responses
typedef struct LoginResponse LoginResponse;
typedef struct AssignResponse AssignResponse;
typedef struct Response Response;

//Requests
typedef struct LoginRequest LoginRequest;
typedef struct AssignRequest AssignRequest;
typedef struct LARequest LARequest;

//Enums
typedef enum RequestType RequestType;

struct LoginResponse{
	TCHAR licensePlate[STRING_SMALL];
	int coordX;
	int coordY;
};

struct AssignResponse{
	int idPassenger;
};

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

enum RequestType { LOGIN, ASSIGN };
