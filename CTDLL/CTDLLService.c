#pragma once
#include "CTDLLService.h"

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->loggedInTaxi.empty = true;
	app->quant = 0;

	return (Setup_OpenSyncHandles(&app->syncHandles) &&
		Setup_OpenSmhHandles(app) &&
		Setup_OpenThreadHandles(app));
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hMutex_LARequest = CreateMutex(//This mutex is only created on and for ConTaxi
		NULL,					//Security attributes
		FALSE,					//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_LARequest	//Mutex name
	);
	syncHandles->hEvent_LARequest_Read = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,			//Desired access flag
		FALSE,						//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_LARequest_Read	//Event name
	);
	syncHandles->hEvent_LARequest_Write = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,			//Desired access flag
		FALSE,						//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_LARequest_Write	//Event name
	);

	syncHandles->hEvent_Notify_T_NP = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_Notify_T_NP			//Event name
	);

	return !(syncHandles->hMutex_LARequest == NULL ||
		syncHandles->hEvent_LARequest_Read == NULL ||
		syncHandles->hEvent_LARequest_Write == NULL ||
		syncHandles->hEvent_Notify_T_NP == NULL);
}

bool Setup_OpenSmhHandles(Application* app){
#pragma region LARequest
	app->shmHandles.hSHM_LARequest = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	//Desired access flag
		FALSE,					//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_LAREQUESTS		//File mapping object name
	);
	if(app->shmHandles.hSHM_LARequest == NULL)
		return false;

	app->shmHandles.lpSHM_LARequest = MapViewOfFile(
		app->shmHandles.hSHM_LARequest,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(LARequest)				//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_LARequest == NULL)
		return false;
#pragma endregion

	return true;
}

bool Setup_OpenThreadHandles(Application* app){
	app->threadHandles.hNotificationReceiver_NP = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_NotificationReceiver_NP,		//Function
		(LPVOID) app,						//Param
		CREATE_SUSPENDED,					//Creation Flag
		&app->threadHandles.dwIdLARequests  //Thread ID
	);

	if(app->threadHandles.hNotificationReceiver_NP == NULL)
		return false;

	return true;
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseSyncHandles(&app->syncHandles);
	Setup_CloseSmhHandles(&app->shmHandles);
	Setup_CloseThreadHandles(&app->threadHandles);
}

void Setup_CloseSyncHandles(SyncHandles* syncHandles){
	CloseHandle(syncHandles->hMutex_LARequest);
	CloseHandle(syncHandles->hEvent_LARequest_Read);
	CloseHandle(syncHandles->hEvent_LARequest_Write);
}

void Setup_CloseSmhHandles(ShmHandles* shmHandles){
#pragma region SendRequest
	UnmapViewOfFile(shmHandles->lpSHM_LARequest);
	CloseHandle(shmHandles->hSHM_LARequest);
#pragma endregion
}

void Setup_CloseThreadHandles(ThreadHandles* threadHandles){
	CloseHandle(threadHandles->hLARequests);
	CloseHandle(threadHandles->hNotificationReceiver_NP);
}
