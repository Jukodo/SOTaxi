#pragma once
#include "CenDLL.h"

typedef struct LoginRequest LoginRequest;
typedef struct AssignRequest AssignRequest;
typedef struct Request Request;
typedef enum RequestType RequestType;

struct LoginRequest{
	TCHAR licensePlate[STRING_SMALL];
	int coordX;
	int coordY;
};

struct AssignRequest{
	int idPassenger;
};

struct Request{
	union{
		LoginRequest loginRequest;
		AssignRequest assignRequest;
	};
};

enum RequestType { LOGIN, ASSIGN };