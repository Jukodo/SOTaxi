#pragma once
#include "CenDLL.h"

typedef struct ThreadHandles{
	HANDLE hLARequests;
	DWORD dwIdLARequests;
}ThreadHandles;

typedef struct SyncHandles{
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
	Taxi* taxiList;
	Passenger* passengerList;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
}Application;

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers);
bool Setup_OpenThreadHandles(Application* app);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
bool Setup_OpenSmhHandles(Application* app);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseSmhHandles(ShmHandles* shmHandles);

bool isTaxiListFull(Application* app);
int Get_QuantLoggedInTaxis(Application* app);
int Get_FreeIndexTaxiList(Application* app);
Taxi* Get_Taxi(Application* app, TCHAR* licensePlate);

bool isPassengerListFull(Application* app);
int Get_QuantLoggedInTaxis(Application* app);
int Get_FreeIndexTaxiList(Application* app);
Passenger* Get_Passenger(Application* app, TCHAR* Id);

bool isValid_ObjectPosition(Application* app, float coordX, float coordY);

LoginResponse Service_LoginTaxi(Application* app, LoginRequest* loginRequest);
bool Service_NewPassenger(Application* app, Passenger pass);
AssignResponse Service_RequestPassenger(Application* app, AssignRequest* assignRequest);
void Service_NotifyTaxisNewPassenger(Application* app);