#pragma once
#include "CenService.h"
#include "CenCommunication.h"

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));

	bool ret = true;
	ret = ret && Setup_OpenSyncHandles(&app->syncHandles);
	ret = ret && Setup_OpenSmhHandles(&app->shmHandles);
	ret = ret && Setup_OpenThreadHandles(app);

	return ret;
}

bool Setup_OpenThreadHandles(Application* app){
	TParam_LARequest* param = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));

	param->app = app;
	app->threadHandles.hLARequests = CreateThread(
			NULL,
			0,
			Thread_ReceiveLARequests,			//Function
			(LPVOID) param,						//Param
			0,									//Creation flags
			&app->threadHandles.dwIdLARequests  //Thread Id
	);
	
	return !(app->threadHandles.hLARequests == NULL);
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hEvent_LARequest_Read = CreateEvent(NULL, FALSE, FALSE, NAME_EVENT_LARequest_Read);
	syncHandles->hEvent_LARequest_Write = CreateEvent(NULL, FALSE, TRUE, NAME_EVENT_LARequest_Write);

	return !(syncHandles->hEvent_LARequest_Read == NULL ||
		syncHandles->hEvent_LARequest_Write == NULL);
}

bool Setup_OpenSmhHandles(ShmHandles* shmHandles){
	shmHandles->hSHM_LARequest = CreateFileMapping
	(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(LARequest),
		SHM_Testing);

	if(shmHandles->hSHM_LARequest == NULL)
		return false;

	shmHandles->lpSHM_LARequest = MapViewOfFile(
		shmHandles->hSHM_LARequest,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(LARequest));

	if(shmHandles->lpSHM_LARequest == NULL)
		return false;
	return true;
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseSyncHandles(&app->syncHandles);
	Setup_CloseSmhHandles(&app->shmHandles);
}

void Setup_CloseSyncHandles(SyncHandles* syncHandles){
	CloseHandle(syncHandles->hEvent_LARequest_Read);
	CloseHandle(syncHandles->hEvent_LARequest_Write);
}

void Setup_CloseSmhHandles(ShmHandles* shmHandles){
#pragma region SendRequest
	UnmapViewOfFile(shmHandles->lpSHM_LARequest);
	CloseHandle(shmHandles->hSHM_LARequest);
#pragma endregion
}