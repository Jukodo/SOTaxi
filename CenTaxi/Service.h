#pragma once
#include "CenDLL.h"

typedef struct Application{
	Taxi* listTaxi;
	//Passenger* listPassenger;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
}Application;

typedef struct ThreadHandles{
	HANDLE thread;
}ThreadHandles;

typedef struct SyncHandles{
	HANDLE event;
}SyncHandles;

typedef struct ShmHandles{
	HANDLE hTestMem;
	LPVOID lpTestMem;
}ShmHandles;