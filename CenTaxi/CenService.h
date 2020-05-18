#pragma once
#include "CenDLL.h"

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;

struct ThreadHandles{
	HANDLE hLARequests;
	DWORD dwIdLARequests;
};

struct SyncHandles{
	HANDLE hEvent_LARequest_Read;
	HANDLE hEvent_LARequest_Write;
	HANDLE hEvent_Notify_T_NewTranspReq;
};

struct ShmHandles{
	HANDLE hSHM_LARequest;
	LPVOID lpSHM_LARequest;
	HANDLE hSHM_NTBuffer;
	LPVOID lpSHM_NTBuffer;
};

struct Application{
	Taxi* taxiList;
	Passenger* passengerList;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
};

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
void Service_NotifyTaxisNewTransport(Application* app);