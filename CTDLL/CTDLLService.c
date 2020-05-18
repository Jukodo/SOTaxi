#pragma once
#include "CTDLLService.h"

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->loggedInTaxi.empty = true;

	return (Setup_OpenSyncHandles(&app->syncHandles) &&
		Setup_OpenSmhHandles(app) &&
		Setup_OpenThreadHandles(app));
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hMutex_QnARequest = CreateMutex(//This mutex is only created on and for ConTaxi
		NULL,					//Security attributes
		FALSE,					//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_QnARequest	//Mutex name
	);
	syncHandles->hEvent_QnARequest_Read = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,			//Desired access flag
		FALSE,						//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Read	//Event name
	);
	syncHandles->hEvent_QnARequest_Write = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,			//Desired access flag
		FALSE,						//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Write	//Event name
	);

	syncHandles->hEvent_Notify_T_NewTranspReq = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_NewTransportRequest	//Event name
	);

	return !(syncHandles->hMutex_QnARequest == NULL ||
		syncHandles->hEvent_QnARequest_Read == NULL ||
		syncHandles->hEvent_QnARequest_Write == NULL ||
		syncHandles->hEvent_Notify_T_NewTranspReq == NULL);
}

bool Setup_OpenSmhHandles(Application* app){
	#pragma region QnARequest
	app->shmHandles.hSHM_QnARequest = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	//Desired access flag
		FALSE,					//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_QnARequest	//File mapping object name
	);
	if(app->shmHandles.hSHM_QnARequest == NULL)
		return false;

	app->shmHandles.lpSHM_QnARequest = MapViewOfFile(
		app->shmHandles.hSHM_QnARequest,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(QnARequest)				//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_QnARequest == NULL)
		return false;
#pragma endregion

	#pragma region NewTransportBuffer
	app->shmHandles.hSHM_NTBuffer = OpenFileMapping(
		FILE_MAP_READ,					//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_TransportRequestBuffer	//File mapping object name
	);
	if(app->shmHandles.hSHM_NTBuffer == NULL)
		return false;

	app->shmHandles.lpSHM_NTBuffer = MapViewOfFile(
		app->shmHandles.hSHM_NTBuffer,	//File mapping object handle
		FILE_MAP_READ,					//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(NewTransportBuffer)		//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_NTBuffer == NULL)
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
		&app->threadHandles.dwIdQnARequests  //Thread ID
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
	CloseHandle(syncHandles->hMutex_QnARequest);
	CloseHandle(syncHandles->hEvent_QnARequest_Read);
	CloseHandle(syncHandles->hEvent_QnARequest_Write);
	CloseHandle(syncHandles->hEvent_Notify_T_NewTranspReq);
}

void Setup_CloseSmhHandles(ShmHandles* shmHandles){
#pragma region SendRequest
	UnmapViewOfFile(shmHandles->lpSHM_QnARequest);
	CloseHandle(shmHandles->hSHM_QnARequest);
#pragma endregion
#pragma region NewTransportBuffer
	UnmapViewOfFile(shmHandles->lpSHM_NTBuffer);
	CloseHandle(shmHandles->hSHM_NTBuffer);
#pragma endregion
}

void Setup_CloseThreadHandles(ThreadHandles* threadHandles){
	CloseHandle(threadHandles->hQnARequests);
	CloseHandle(threadHandles->hNotificationReceiver_NP);
}
