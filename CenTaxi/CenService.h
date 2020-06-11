#pragma once
#include "CenDLL.h"
#include "CenTaxi.h"
#include "CenPassenger.h"
#include "CenSettings.h"
#include "CenThreads.h"

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;
typedef enum CentralCommands CentralCommands;

struct ThreadHandles{
	HANDLE hQnARequests;
	DWORD dwIdQnARequests;
	HANDLE hTossRequests;
	DWORD dwIdTossRequests;
	HANDLE hConnectingTaxiPipes;
	DWORD dwIdConnectingTaxiPipes;
};

struct SyncHandles{
	HANDLE hEvent_QnARequest_Read;
	HANDLE hEvent_QnARequest_Write;
	HANDLE hEvent_Notify_T_NewTranspReq;
	HANDLE hEvent_NewTaxiSpot;
	HANDLE hSemaphore_HasTossRequest;
	HANDLE hSemaphore_TaxiNPSpots;
};

struct ShmHandles{
	HANDLE hSHM_QnARequest;
	LPVOID lpSHM_QnARequest;
	HANDLE hSHM_NTBuffer;
	LPVOID lpSHM_NTBuffer;
	HANDLE hSHM_Map;
	LPVOID lpSHM_Map;
	HANDLE hSHM_TossReqBuffer;
	LPVOID lpSHM_TossReqBuffer;
};

struct Application{
	Settings settings;
	CenTaxi* taxiList;
	Map map;
	CenPassenger* passengerList;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
	bool keepRunning; //Used to identify if app should keep running, when FALSE threads that are infinitely looping will end //ToDo (TAG_TODO): Threads not closing properly
};

#define CMD_HELP TEXT("/help")
#define CMD_LIST_TAXIS TEXT("/listTaxis")
#define CMD_SET_TIMEOUT TEXT("/setTimeout")
#define CMD_TAXI_LOGIN_ON TEXT("/taxiLoginOn")
#define CMD_TAXI_LOGIN_OFF TEXT("/taxiLoginOff")
#define CMD_KICK_TAXI TEXT("/kickTaxi")
#define CMD_SIMULATE_NTR TEXT("/simulateNTR")
#define CMD_SHOW_MAP TEXT("/showMap")
#define CMD_SAVE_REGISTRY TEXT("/saveRegistry")
#define CMD_LOAD_REGISTRY TEXT("/loadRegistry")
#define CMD_DLL_LOG TEXT("/createDllLog")
#define CMD_ASSIGN_RANDOM TEXT("/assignRandom")
#define CMD_SHUTDOWN_RANDOM TEXT("/shutdownRandom")
#define CMD_CLOSEAPP TEXT("/closeApp")

enum CentralCommands{
	CC_HELP,
	CC_LIST_TAXIS,
	CC_SET_TIMEOUT,
	CC_TAXI_LOGIN_ON,
	CC_TAXI_LOGIN_OFF,
	CC_KICK_TAXI,
	CC_SIMULATE_NTR,
	CC_SHOW_MAP,
	CC_SAVE_REGISTRY,
	CC_LOAD_REGISTRY,
	CC_DLL_LOG,
	CC_CLOSEAPP,
	CC_ASSIGN_RANDOM,
	CC_SHUTDOWN_RANDOM,
	CC_UNDEFINED
};

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers);
bool Setup_OpenThreadHandles(Application* app);
bool Setup_OpenSyncHandles(Application* app);
bool Setup_OpenShmHandles(Application* app);
bool Setup_OpenShmHandles_Map(Application* app);
bool Setup_OpenMap(Application* app);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseShmHandles(ShmHandles* shmHandles);

bool isTaxiListFull(Application* app);
bool Add_Taxi(Application* app, TCHAR* licensePlate, double coordX, double coordY);
bool Delete_Taxi(Application* app, int index);
int Get_QuantLoggedInTaxis(Application* app);
int Get_FreeIndexTaxiList(Application* app);
int Get_TaxiIndex(Application* app, TCHAR* licensePlate);
CenTaxi* Get_Taxi(Application* app, int index);
CenTaxi* Get_TaxiAt(Application* app, int coordX, int coordY);

bool isPassengerListFull(Application* app);
int Get_QuantLoggedInTaxis(Application* app);
int Get_FreeIndexTaxiList(Application* app);
int Get_PassengerIndex(Application* app, TCHAR* Id);
CenPassenger* Get_Passenger(Application* app, int index);

bool isValid_ObjectPosition(Application* app, double coordX, double coordY);

CentralCommands Service_UseCommand(Application* app, TCHAR* command);

LoginResponseType Service_LoginTaxi(Application* app, LoginRequest* loginRequest);
bool Service_NewPassenger(Application* app, Passenger pass);
NTInterestResponse Service_RegisterInterest(Application* app, NTInterestRequest* ntiRequest);
void Service_NotifyTaxisNewTransport(Application* app); 
bool Service_AssignTaxi2Passenger(Application* app, int taxiIndex, int passengerIndex);
bool Service_KickTaxi(Application* app, TCHAR* licensePlate, TCHAR* reason, bool global);
void Service_CloseApp(Application* app);

bool Command_SetAssignmentTimeout(Application* app, TCHAR* value);
void Command_AllowTaxiLogins(Application* app, bool allow);

/* ToDo (TAG_REMOVE)
** Remove the following after
** Only used to develop and test few features
*/
void Simulate_NewTransport(Application* app);
void Temp_ShowMap(Application* app);
void Temp_SaveRegistry(Application* app);
void Temp_LoadRegistry(Application* app);
void Temp_AssignRandom(Application* app);
void Temp_ShutdownRandom(Application* app);