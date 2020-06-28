#pragma once
#include "CenDLL.h"

#define NAME_MUTEX_QnARequest TEXT("JUSO2TAXI_MUTEX_QNAR")

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;

struct ThreadHandles{
	HANDLE placeholder;
};

struct SyncHandles{
	HANDLE hMutex_QnARequest_CanAccess;
	HANDLE hEvent_QnARequest_Read;
	HANDLE hEvent_QnARequest_Write;
};

struct ShmHandles{
	HANDLE hSHM_QnARequest;
	HANDLE lpSHM_QnARequest;
	HANDLE hSHM_Map;
	HANDLE lpSHM_Map;
};

struct Application{
	Map map;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	int maxTaxis;
	int maxPassengers;
};

bool Setup_Application(Application* app);
bool Setup_OpenThreadHandles(Application* app);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
bool Setup_OpenShmHandles(Application* app);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseShmHandles(ShmHandles* shmHandles);

bool Service_RequestMaxVars(Application* app);