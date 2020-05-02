#pragma once
#include "CenDLL.h"

typedef struct ThreadHandles{
	HANDLE hSendRequests;
	DWORD dwIdSendRequests;
}ThreadHandles;

typedef struct SyncHandles{
	HANDLE mutex_SendRequest;
	int a;
}SyncHandles;

typedef struct ShmHandles{
	HANDLE hTestMem;
	LPVOID lpTestMem;
}ShmHandles;

typedef struct Application{
	Taxi loggedInTaxi;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
}Application;

bool isLoggedIn(Application* app);
bool isValid_LicensePlate(TCHAR* sLicensePlate);
bool isValid_Coordinates(TCHAR* sCoordinates);

bool Setup_Application(Application* app);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
bool Setup_OpenSmhHandles(ShmHandles* shmHandles);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseSmhHandles(ShmHandles* shmHandles);

//void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y);