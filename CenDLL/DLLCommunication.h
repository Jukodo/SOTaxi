#pragma once
#include "CenDLL.h"

typedef enum ShutdownType ShutdownType;

#pragma region Shared Memory - Structs/Enums Prototypes
//QnARequest Requests Variants
typedef struct TaxiLoginRequest TaxiLoginRequest;
typedef struct NTInterestRequest NTInterestRequest;

//QnARequest Responses Variants
typedef struct TaxiLoginResponse TaxiLoginResponse;
typedef enum TaxiLoginResponseType TaxiLoginResponseType;
typedef enum TransportInterestResponse TransportInterestResponse;
typedef struct MaxVarsResponse MaxVarsResponse;

//QnARequest Types Enums
typedef enum QnARequestType QnARequestType;

//TossRequest Variants
typedef struct TossPosition TossPosition;
typedef struct TossSpeed TossSpeed;
typedef struct TossInterest TossInterest;
typedef struct TossState TossState;
typedef struct TossLogout TossLogout;

//TossRequest Types Enums
typedef enum TossRequestType TossRequestType;

//Core Structs
typedef struct QnARequest QnARequest;
typedef struct TossRequest TossRequest;

//Aditional Structs
typedef struct TransportRequest TransportRequest;

//Circular Buffers
typedef struct TransportBuffer TransportBuffer;
typedef struct TossRequestsBuffer TossRequestsBuffer;
#pragma endregion

#pragma region Named Pipe - Structs/Enums Prototypes
//Core Structs
typedef struct CommsC2T CommsC2T; //NamedPipe comms types from CenTaxi to ConTaxi
typedef struct CommsT2C CommsT2C; //NamedPipe comms types from ConTaxi to CenTaxi
typedef struct CommsC2P CommsC2P; //NamedPipe comms types from CenTaxi to ConPass
typedef struct CommsP2C CommsP2C; //NamedPipe comms types from ConPass to CenTaxi

//CommsC2T Variants
typedef struct CommsC2T_Assign CommsC2T_Assign;
typedef struct CommsC2T_Shutdown CommsC2T_Shutdown;

//CommsT2C Variants
typedef struct CommsT2C_Identity CommsT2C_Identity;

//CommsC2P Variants
typedef struct CommsC2P_Assign CommsC2P_Assign;
typedef struct CommsC2P_PassRemoval CommsC2P_PassRemoval;
typedef struct CommsC2P_PassArrived CommsC2P_PassArrived;
typedef struct CommsC2P_Shutdown CommsC2P_Shutdown;
typedef struct CommsC2P_Resp_MaxPass CommsC2P_Resp_MaxPass;
typedef enum CommsC2P_Resp_Login CommsC2P_Resp_Login;

//CommsP2C Variants
typedef struct CommsP2C_Login CommsP2C_Login;

//Comm Types Enums
typedef enum CommsC2TType CommsC2TType; //NamedPipe comms types from CenTaxi to ConTaxi
typedef enum CommsT2CType CommsT2CType; //NamedPipe comms types from ConTaxi to CenTaxi
typedef enum CommsC2PType CommsC2PType; //NamedPipe comms types from CenTaxi to ConPass
typedef enum CommsP2CType CommsP2CType; //NamedPipe comms types from ConPass to CenTaxi
#pragma endregion

enum Shutdown_Type{
	ST_GLOBAL,
	ST_KICKED
};

#pragma region Shared Memory - Structs/Enums Definement
//QnARequest Requests Variants
struct TaxiLoginRequest{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	XY xyStartingPosition;
};
struct NTInterestRequest{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	TCHAR idPassenger[STRING_ID];
};

//QnARequest Responses Variants
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
struct MaxVarsResponse{
	int maxTaxis;
	int maxPassengers;
	int mapWidth;
	int mapHeight;
};

//QnARequest Types Enums
enum QnARequestType{ //Types of QnA requests 
	QnART_LOGIN, //Taxi login
	QnART_NT_INTEREST, //Taxi interest in a transport request
	QnART_MAX_VARS //MapInfo requests max taxi and max passenger to create taxi/pass list map view of file
};

//TossRequest Variants
struct TossPosition{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	XY xyNewPosition;
};
struct TossSpeed{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	double newSpeed;
};
struct TossState{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	TaxiState newState;
};
struct TossInterest{
	TCHAR licensePlate[STRING_LICENSEPLATE];
	TCHAR idPassenger[STRING_ID];
};
struct TossLogout{
	TCHAR licensePlate[STRING_LICENSEPLATE];
};

//TossRequest Types Enums
enum TossRequestType{ //Types of Toss requests 
	TRT_TAXI_POSITION, //Taxi new position
	TRT_TAXI_SPEED, //Taxi new speed
	TRT_TAXI_STATE, //Taxi new state
	TRT_TAXI_INTEREST, //Taxi transport interest
	TRT_TAXI_LOGOUT //Taxi logout
};

//Core Structs
struct QnARequest{
	union{
		TaxiLoginRequest taxiLoginRequest;
		NTInterestRequest ntIntRequest;
	};
	QnARequestType requestType;
	union{
		TaxiLoginResponse taxiLoginResponse;
		TransportInterestResponse ntIntResponse;
		MaxVarsResponse maxVarsResponse;
	};
};
struct TossRequest{
	union{
		TossPosition tossPosition;
		TossSpeed tossSpeed;
		TossState tossState;
		TossInterest tossInterest;
		TossLogout tossLogout;
	};
	TossRequestType tossType;
};

//Additional Structs
struct TransportRequest{
	bool empty;
	TCHAR passId[STRING_ID];
	XY xyStartingPosition;
	XY xyDestination;
};

//Circular Buffers
struct TransportBuffer{
	TransportRequest transportRequests[NTBUFFER_MAX];
	int head;
};
struct TossRequestsBuffer{
	TossRequest tossRequests[TOSSBUFFER_MAX];
	int head;
	int tail;
};
#pragma endregion

#pragma region Named Pipe - Structs/Enums Definement
//CommsC2T Variants
struct CommsC2T_Assign{
	TransportRequest transportInfo;
};
struct CommsC2T_Shutdown{
	ShutdownType shutdownType;
	TCHAR message[STRING_LARGE];
};

//CommsT2C Variants
struct CommsT2C_Identity{
	TCHAR licensePlate[STRING_LICENSEPLATE];
};

//CommsC2P Variants
struct CommsC2P_Assign{
	TCHAR passId[STRING_ID];
	TCHAR licensePlate[STRING_LICENSEPLATE];
	int estimatedWaitTime;
};
struct CommsC2P_PassRemoval{
	TCHAR passId[STRING_ID];
};
struct CommsC2P_PassArrived{
	TCHAR passId[STRING_ID];
};
struct CommsC2P_Shutdown{
	TCHAR message[STRING_LARGE];
};
struct CommsC2P_Resp_MaxPass{
	int maxPass;
};
enum CommsC2P_Resp_Login{
	PLR_SUCCESS,
	PLR_INVALID_UNDEFINED,
	PLR_INVALID_TRANSPBUFFER_FULL,
	PLR_INVALID_FULL,
	PLR_INVALID_POSITION,
	PLR_INVALID_DESTINY,
	PLR_INVALID_EXISTS
};

//CommsP2C Variants
struct CommsP2C_Login{
	TCHAR id[STRING_ID];
	XY xyStartingPosition;
	XY xyDestination;
};

//Comm Types Enums
enum CommsC2TType{
	C2T_ASSIGNED,
	C2T_SHUTDOWN
};
enum CommsT2CType{
	T2C_IDENTITY
};
enum CommsC2PType{
	C2P_ASSIGNED,
	C2P_ASSIGNED_FAILED,
	C2P_PASS_REMOVAL,
	C2P_PASS_ARRIVED,
	C2P_RESP_LOGIN,
	C2P_RESP_MAXPASS,
	C2P_SHUTDOWN
};
enum CommsP2CType{
	P2C_LOGIN,
	P2C_REQMAXPASS, //This type doesn't hold a struct to send info to central, only informs central to send maxPass to ConPass
	P2C_DISCONNECT //This type doesn't hold a struct to send info to central, only informs central that ConPass intends to disconnect
}; 

//Core Structs
struct CommsC2T{
	union{
		CommsC2T_Assign assignComm;
		CommsC2T_Shutdown shutdownComm;
	};
	CommsC2TType commType;
};
struct CommsT2C{
	union{//Unecessary but ready to implement more
		CommsT2C_Identity identityComm;
	};
	CommsT2CType commType;
};
struct CommsC2P{
	union{
		CommsC2P_Assign assignComm;
		CommsC2P_PassRemoval removeComm;
		CommsC2P_PassArrived arrivedComm;
		CommsC2P_Shutdown shutdownComm;
		CommsC2P_Resp_Login loginRespComm;
		CommsC2P_Resp_MaxPass maxPassRespComm;
	};
	CommsC2PType commType;
};
struct CommsP2C{
	union{//Unecessary but ready to implement more
		CommsP2C_Login loginComm;
	};
	CommsP2CType commType;
};
#pragma endregion
