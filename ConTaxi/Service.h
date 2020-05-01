#pragma once
#include "CenDLL.h"

typedef struct ThreadHandles{
	int Id;
	HANDLE thread;
}ThreadHandles;

typedef struct SyncHandles{
	int Id;
	HANDLE event;
}SyncHandles;

typedef struct ShmHandles{
	int Id;
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

bool Setup_Application(Application* app);

bool Setup_OpenThreadHandles(ThreadHandles* threadHandles);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
bool Setup_OpenSmhHandles(ShmHandles* shmHandles);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseThreadHandles(ThreadHandles* threadHandles);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseSmhHandles(ShmHandles* shmHandles);