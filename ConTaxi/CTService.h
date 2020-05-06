#pragma once
#include "CenDLL.h"

typedef struct ThreadHandles{
	HANDLE hLARequests;
	DWORD dwIdLARequests;
	HANDLE hNotificationReceiver_NP;
	DWORD dwIdNotificationReceiver_NP;
}ThreadHandles;

typedef struct SyncHandles{
	HANDLE hMutex_LARequest;
	HANDLE hEvent_LARequest_Read;
	HANDLE hEvent_LARequest_Write;
	HANDLE hEvent_PassengerList_Access;
	HANDLE hEvent_Notify_T_NP;
}SyncHandles;

typedef struct ShmHandles{
	HANDLE hSHM_LARequest;
	LPVOID lpSHM_LARequest;
	HANDLE hSHM_PassengerList;
	LPVOID lpSHM_PassengerList;
}ShmHandles;

typedef struct Application{
	Taxi loggedInTaxi;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
	int quant;
}Application;

bool isLoggedIn(Application* app);
bool isValid_LicensePlate(TCHAR* sLicensePlate);
bool isValid_Coordinates(TCHAR* sCoordinates);

bool Setup_Application(Application* app);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
bool Setup_OpenSmhHandles(Application* app);
bool Setup_OpenThreadHandles(Application* app);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseSmhHandles(ShmHandles* shmHandles);
void Setup_CloseThreadHandles(ThreadHandles* threadHandles);

void Service_RequestVars(Application* app);
void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y);
TaxiCommands Service_UseCommand(Application* app, TCHAR* command);
void Service_RequestPass(Application* app, TCHAR* idPassenger);
bool Service_DefineCDN(Application* app, TCHAR* value);

void Command_ListPassengers(Application* app);
void Command_Speed(Application* app, bool speedUp);
void Command_AutoResp(Application* app, bool autoResp);
void Command_CloseApp(Application* app);