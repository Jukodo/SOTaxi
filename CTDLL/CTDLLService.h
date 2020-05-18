#pragma once
#include "CTDLL.h"

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;

struct ThreadHandles{
	HANDLE hQnARequests;
	DWORD dwIdQnARequests;
	HANDLE hNotificationReceiver_NP;
	DWORD dwIdNotificationReceiver_NP;
};

struct SyncHandles{
	HANDLE hMutex_QnARequest;
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
	Taxi loggedInTaxi;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
	int NTBuffer_Tail; //Used as a tail for the new transport request buffer (each taxi has a tail)
};

CTDLL_API bool Setup_Application(Application* app);
CTDLL_API bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
CTDLL_API bool Setup_OpenSmhHandles(Application* app);
CTDLL_API bool Setup_OpenThreadHandles(Application* app);

CTDLL_API void Setup_CloseAllHandles(Application* app);
CTDLL_API void Setup_CloseSyncHandles(SyncHandles* syncHandles);
CTDLL_API void Setup_CloseSmhHandles(ShmHandles* shmHandles);
CTDLL_API void Setup_CloseThreadHandles(ThreadHandles* threadHandles);