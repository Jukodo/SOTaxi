#pragma once
#include "CenDLL.h"
#include "CenPassenger.h"
#include "CenSettings.h"

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;
typedef enum CentralCommands CentralCommands;

struct ThreadHandles{
	HANDLE hQnARequests;
	DWORD dwIdQnARequests;
};

struct SyncHandles{
	HANDLE hEvent_QnARequest_Read;
	HANDLE hEvent_QnARequest_Write;
	HANDLE hEvent_Notify_T_NewTranspReq;
};

struct ShmHandles{
	HANDLE hSHM_QnARequest;
	LPVOID lpSHM_QnARequest;
	HANDLE hSHM_NTBuffer;
	LPVOID lpSHM_NTBuffer;
};

struct Application{
	Settings settings;
	Taxi* taxiList;
	CenPassenger* passengerList;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
};

#define CMD_HELP TEXT("/help")
#define CMD_LIST_TAXIS TEXT("/listTaxis")
#define CMD_SET_TIMEOUT TEXT("/setTimeout")
#define CMD_TAXI_LOGIN_ON TEXT("/taxiLoginOn")
#define CMD_TAXI_LOGIN_OFF TEXT("/taxiLoginOff")
#define CMD_KICK_TAXI TEXT("/kickTaxi")
#define CMD_SIMULATE_NTR TEXT("/simulateNTR")
#define CMD_CLOSEAPP TEXT("/closeApp")

enum CentralCommands{
	CC_HELP,
	CC_LIST_TAXIS,
	CC_SET_TIMEOUT,
	CC_TAXI_LOGIN_ON,
	CC_TAXI_LOGIN_OFF,
	CC_KICK_TAXI,
	CC_SIMULATE_NTR,
	CC_CLOSEAPP,
	CC_UNDEFINED
};

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers);
bool Setup_OpenThreadHandles(Application* app);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
bool Setup_OpenSmhHandles(Application* app);
bool Setup_OpenMap(Application* app);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseSmhHandles(ShmHandles* shmHandles);

bool isTaxiListFull(Application* app);
int Get_QuantLoggedInTaxis(Application* app);
int Get_FreeIndexTaxiList(Application* app);
int Get_TaxiIndex(Application* app, TCHAR* licensePlate);
Taxi* Get_Taxi(Application* app, int index);

bool isPassengerListFull(Application* app);
int Get_QuantLoggedInTaxis(Application* app);
int Get_FreeIndexTaxiList(Application* app);
int Get_PassengerIndex(Application* app, TCHAR* Id);
CenPassenger* Get_Passenger(Application* app, int index);

bool isValid_ObjectPosition(Application* app, float coordX, float coordY);

CentralCommands Service_UseCommand(Application* app, TCHAR* command);

LoginResponse Service_LoginTaxi(Application* app, LoginRequest* loginRequest);
bool Service_NewPassenger(Application* app, Passenger pass);
NTInterestResponse Service_RegisterInterest(Application* app, NTInterestRequest* ntiRequest);
void Service_NotifyTaxisNewTransport(Application* app); 
bool Service_KickTaxi(Application* app, TCHAR* licensePlate);
void Service_CloseApp(Application* app);

bool Command_SetAssignmentTimeout(Application* app, TCHAR* value);
void Command_AllowTaxiLogins(Application* app, bool allow);
void Simulate_NewTransport(Application* app);