#pragma once
#include "CenDLL.h"
#include "CenTaxi.h"
#include "CenPassenger.h"
#include "CenTransportBuffer.h"
#include "CenSettings.h"
#include "CenThreads.h"

typedef struct Application Application;
typedef struct NamedPipeHandles NamedPipeHandles;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;
typedef enum CentralCommands CentralCommands;

struct NamedPipeHandles{
	HANDLE hCPWrite;
	HANDLE hCPRead;
	HANDLE hCPQnA;
};

struct ThreadHandles{
	HANDLE hQnARequests;
	DWORD dwIdQnARequests;
	HANDLE hTossRequests;
	DWORD dwIdTossRequests;
	HANDLE hConnectingTaxiPipes;
	DWORD dwIdConnectingTaxiPipes;
	HANDLE hReadConPassNPQnA;
	DWORD dwIdReadConPassNPQnA;
	HANDLE hReadConPassNPToss;
	DWORD dwIdReadConPassNPToss;
};

struct SyncHandles{
	HANDLE hEvent_TaxiList;
	HANDLE hEvent_PassengerList;
	HANDLE hEvent_QnARequest_Read;
	HANDLE hEvent_QnARequest_Write;
	HANDLE hEvent_Notify_T_NewTranspReq;
	HANDLE hEvent_NewTaxiSpot;
	HANDLE hSemaphore_HasTossRequest;
	HANDLE hSemaphore_TaxiNPSpots;
};

struct ShmHandles{
	HANDLE hSHM_TaxiList;
	LPVOID lpSHM_TaxiList;
	HANDLE hSHM_PassengerList;
	LPVOID lpSHM_PassengerList;
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
	Map map;
	CenTaxi* taxiList;
	CenPassenger* passengerList;

	/*Shared list is on shared memory
	**This list contains "NTBUFFER_MAX" items
	**Each item contains a list of interested taxis, up to app->maxTaxis
	**	Also contains handle and id of thread to assign the transport to a taxi
	**Both lists are linked by using the same index
	*/CenTransportRequest* transportList;

	NamedPipeHandles namedPipeHandles;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
	bool keepRunning; //Used to identify if app should keep running, when FALSE threads that are infinitely looping will end //ToDo (TAG_TODO): Threads not closing properly
};

#define CMD_HELP TEXT("/help")
#define CMD_LIST_TAXIS TEXT("/listTaxis")
#define CMD_LIST_PASS TEXT("/listPass")
#define CMD_SET_TIMEOUT TEXT("/setTimeout")
#define CMD_TAXI_LOGIN_ON TEXT("/taxiLoginOn")
#define CMD_TAXI_LOGIN_OFF TEXT("/taxiLoginOff")
#define CMD_KICK_TAXI TEXT("/kickTaxi")
#define CMD_SHOW_MAP TEXT("/showMap")
#define CMD_SAVE_REGISTRY TEXT("/saveRegistry")
#define CMD_LOAD_REGISTRY TEXT("/loadRegistry")
#define CMD_DLL_LOG TEXT("/createDllLog")
#define CMD_CLOSEAPP TEXT("/closeApp")

#define NAME_MUTEX_OneInstance_CEN TEXT("JUSO2TAXI_MUTEX_OI_CEN")

enum CentralCommands{
	CC_HELP,
	CC_LIST_TAXIS,
	CC_LIST_PASS,
	CC_SET_TIMEOUT,
	CC_TAXI_LOGIN_ON,
	CC_TAXI_LOGIN_OFF,
	CC_KICK_TAXI,
	CC_SHOW_MAP,
	CC_SAVE_REGISTRY,
	CC_LOAD_REGISTRY,
	CC_DLL_LOG,
	CC_CLOSEAPP,
	CC_UNDEFINED
};

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers);
bool Setup_OpenNamedPipes(NamedPipeHandles* namedPipeHandles);
bool Setup_OpenThreadHandles(Application* app);
bool Setup_OpenSyncHandles(Application* app);
bool Setup_OpenShmHandles(Application* app);
bool Setup_OpenShmHandles_Map(Application* app);
bool Setup_OpenMap(Application* app);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseShmHandles(ShmHandles* shmHandles);

bool Add_Taxi(Application* app, TCHAR* licensePlate, XY xyStartingPosition);
bool Delete_Taxi(Application* app, int index);
int Get_QuantLoggedInTaxis(Application* app);
bool isTaxiListFull(Application* app);
int Get_FreeIndexTaxiList(Application* app);
int Get_TaxiIndex(Application* app, TCHAR* licensePlate);
CenTaxi* Get_Taxi(Application* app, int index);
CenTaxi* Get_TaxiAt(Application* app, XY xyPosition);

bool Add_Passenger(Application* app, TCHAR* id, XY xyStartingPosition, XY xyDestination);
bool Delete_Passenger(Application* app, int index);
int Get_QuantLoggedInPassengers(Application* app);
bool isPassengerListFull(Application* app);
int Get_FreeIndexPassengerList(Application* app);
int Get_PassengerIndex(Application* app, TCHAR* Id);
CenPassenger* Get_Passenger(Application* app, int index);

int Get_TransportIndex(Application* app, TCHAR* idPassenger);
TransportRequest Get_TransportRequest(Application* app, int index);

bool isValid_ObjectPosition(Application* app, XY position, bool ignoreStructures);

CentralCommands Service_UseCommand(Application* app, TCHAR* command);

TaxiLoginResponseType Service_LoginTaxi(Application* app, TaxiLoginRequest* loginRequest);
CommsC2P_Resp_Login Service_LoginPass(Application* app, CommsP2C_Login* loginRequest);
bool Service_NewTransportRequest(Application* app, TransportRequest transportReq);
TransportInterestResponse Service_RegisterInterest(Application* app, NTInterestRequest* ntiRequest);
void Service_NotifyTaxisNewTransport(Application* app); 
void Service_AssignTaxi2Passenger(Application* app, int taxiIndex, int passengerIndex);
void Service_NotifyTaxi(Application* app, TransportRequest* requestInfo, int taxiIndex);
void Service_NotifyPassenger(Application* app, TransportRequest* myRequestInfo, int taxiIndex);
bool Service_KickTaxi(Application* app, TCHAR* licensePlate, TCHAR* reason, bool global);
void Service_CloseApp(Application* app);

bool Command_SetAssignmentTimeout(Application* app, TCHAR* value);
void Command_AllowTaxiLogins(Application* app, bool allow);

/* ToDo (TAG_REMOVE)
** Remove the following after
** Only used to develop and test few features
*/
void Temp_ShowMap(Application* app);
void Temp_SaveRegistry(Application* app);
void Temp_LoadRegistry(Application* app);
