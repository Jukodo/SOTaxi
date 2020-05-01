#include "Service.h"

bool isLoggedIn(Application* app){
	return !app->loggedInTaxi.deleted;
}

bool Setup_Application(Application* app){
	app->loggedInTaxi.deleted = true;

	return (Setup_OpenThreadHandles(&app->threadHandles) &&
		Setup_OpenSyncHandles(&app->syncHandles) &&
		Setup_OpenSmhHandles(&app->shmHandles));
}

bool Setup_OpenThreadHandles(ThreadHandles* threadHandles){
	//ToDo
	return true;
}
bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	//ToDo
	return true;
}
bool Setup_OpenSmhHandles(ShmHandles* shmHandles){
	shmHandles->Id = 3;
	shmHandles->hTestMem = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		SHM_Testing);

	if(shmHandles->hTestMem == NULL){
		_tprintf(TEXT("ERROR: OpenFileMapping failed\tCentral might be closed"));
		return false;
	}

	shmHandles->lpTestMem = MapViewOfFile(
		shmHandles->hTestMem,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(Taxi));

	if(shmHandles->lpTestMem == NULL){
		_tprintf(TEXT("ERROR: MapViewOfFile failed\tCentral might be closed"));
		return false;
	}

	return true;
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseThreadHandles(&app->threadHandles);
	Setup_CloseSyncHandles(&app->syncHandles);
	Setup_CloseSmhHandles(&app->shmHandles);
}

void Setup_CloseThreadHandles(ThreadHandles* threadHandles){
	//ToDo
}

void Setup_CloseSyncHandles(SyncHandles* syncHandles){
	//ToDo
}

void Setup_CloseSmhHandles(ShmHandles* shmHandles){
	#pragma region Testing_SHM
	UnmapViewOfFile(shmHandles->lpTestMem);
	CloseHandle(shmHandles->hTestMem);
	#pragma endregion
}