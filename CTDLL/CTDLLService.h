#pragma once
#include "CTDLL.h"

typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;
typedef struct Application Application;

struct ThreadHandles{
	HANDLE hLARequests;
	DWORD dwIdLARequests;
	HANDLE hNotificationReceiver_NP;
	DWORD dwIdNotificationReceiver_NP;
};

struct SyncHandles{
	HANDLE hMutex_LARequest;
	HANDLE hEvent_LARequest_Read;
	HANDLE hEvent_LARequest_Write;
	HANDLE hEvent_Notify_T_NP;
};

struct ShmHandles{
	HANDLE hSHM_LARequest;
	LPVOID lpSHM_LARequest;
};

struct Application{
	Taxi loggedInTaxi;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
	int quant;
};


CTDLL_API bool Setup_Application(Application* app);
CTDLL_API bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
CTDLL_API bool Setup_OpenSmhHandles(Application* app);
CTDLL_API bool Setup_OpenThreadHandles(Application* app);

CTDLL_API void Setup_CloseAllHandles(Application* app);
CTDLL_API void Setup_CloseSyncHandles(SyncHandles* syncHandles);
CTDLL_API void Setup_CloseSmhHandles(ShmHandles* shmHandles);
CTDLL_API void Setup_CloseThreadHandles(ThreadHandles* threadHandles);