#pragma once
#include "CenDLL.h"
#include "CenPassenger.h"

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;

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
	Taxi* taxiList;
	CenPassenger* passengerList;
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
int Get_TaxiIndex(Application* app, TCHAR* licensePlate);
Taxi* Get_Taxi(Application* app, int index);

bool isPassengerListFull(Application* app);
int Get_QuantLoggedInTaxis(Application* app);
int Get_FreeIndexTaxiList(Application* app);
int Get_PassengerIndex(Application* app, TCHAR* Id);
CenPassenger* Get_Passenger(Application* app, int index);

bool isValid_ObjectPosition(Application* app, float coordX, float coordY);

LoginResponse Service_LoginTaxi(Application* app, LoginRequest* loginRequest);
bool Service_NewPassenger(Application* app, Passenger pass);
NTInterestResponse Service_RegisterInterest(Application* app, NTInterestRequest* ntiRequest);
void Service_NotifyTaxisNewTransport(Application* app);