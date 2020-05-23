#pragma once
#include "CTDLLService.h"

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->loggedInTaxi.empty = true;
	app->loggedInTaxi.object.speedMultiplier = DEFAULT_SPEED;
	app->settings.CDN = DEFAULT_CDN;

	app->taxiMovementRoutine = CreateWaitableTimer(NULL, FALSE, NAME_WAITTIMER_StepRoutine);
	Utils_DLL_Register(NAME_WAITTIMER_StepRoutine, DLL_TYPE_WAITABLETIMER);

	return (app->taxiMovementRoutine != NULL &&
		Setup_OpenSyncHandles(&app->syncHandles) &&
		Setup_OpenSmhHandles(app) &&
		Setup_OpenThreadHandles(app));
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hMutex_QnARequest = CreateMutex(//This mutex is only created on and for ConTaxi
		NULL,							//Security attributes
		FALSE,							//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_QnARequest			//Mutex name
	);
	Utils_DLL_Register(NAME_MUTEX_QnARequest, DLL_TYPE_MUTEX);
	syncHandles->hEvent_QnARequest_Read = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Read		//Event name
	);
	Utils_DLL_Register(NAME_EVENT_QnARequest_Read, DLL_TYPE_EVENT);
	syncHandles->hEvent_QnARequest_Write = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Write		//Event name
	);
	Utils_DLL_Register(NAME_EVENT_QnARequest_Write, DLL_TYPE_EVENT);

	syncHandles->hEvent_Notify_T_NewTranspReq = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_NewTransportRequest	//Event name
	);
	Utils_DLL_Register(NAME_EVENT_NewTransportRequest, DLL_TYPE_EVENT);

	syncHandles->hMutex_TossRequest_CanAccess = CreateMutex(//This mutex is only created on and for ConTaxi
		NULL,							//Security attributes
		FALSE,							//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_TossRequest			//Mutex name
	);
	Utils_DLL_Register(NAME_MUTEX_TossRequest, DLL_TYPE_MUTEX);
	syncHandles->hSemaphore_HasTossRequest = OpenSemaphore(
		SEMAPHORE_ALL_ACCESS,			//Security Attributes
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SEMAPHORE_HasTossRequest	//Semaphore Name
	);
	Utils_DLL_Register(NAME_SEMAPHORE_HasTossRequest, DLL_TYPE_SEMAPHORE);

	return !(syncHandles->hMutex_QnARequest == NULL ||
		syncHandles->hEvent_QnARequest_Read == NULL ||
		syncHandles->hEvent_QnARequest_Write == NULL ||
		syncHandles->hEvent_Notify_T_NewTranspReq == NULL ||
		syncHandles->hMutex_TossRequest_CanAccess == NULL ||
		syncHandles->hSemaphore_HasTossRequest == NULL);
}

bool Setup_OpenSmhHandles(Application* app){
	#pragma region QnARequest
	app->shmHandles.hSHM_QnARequest = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	//Desired access flag
		FALSE,					//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_QnARequest	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_QnARequest == NULL)
		return false;

	app->shmHandles.lpSHM_QnARequest = MapViewOfFile(
		app->shmHandles.hSHM_QnARequest,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(QnARequest)				//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_QnARequest == NULL)
		return false;
	#pragma endregion

	#pragma region NewTransportBuffer
	app->shmHandles.hSHM_NTBuffer = OpenFileMapping(
		FILE_MAP_READ,					//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_TransportRequestBuffer	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_TransportRequestBuffer, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_NTBuffer == NULL)
		return false;

	app->shmHandles.lpSHM_NTBuffer = MapViewOfFile(
		app->shmHandles.hSHM_NTBuffer,	//File mapping object handle
		FILE_MAP_READ,					//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(NewTransportBuffer)		//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_TransportRequestBuffer, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_NTBuffer == NULL)
		return false;
	#pragma endregion

	#pragma region Map
	app->shmHandles.hSHM_Map = OpenFileMapping(
		FILE_MAP_READ,					//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_Map	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_Map == NULL)
		return false;

	app->shmHandles.lpSHM_Map = MapViewOfFile(
		app->shmHandles.hSHM_Map,	//File mapping object handle
		FILE_MAP_READ,				//Desired access flag
		0,							//DWORD high-order of the file offset where the view begins
		0,							//DWORD low-order of the file offset where the view begins
		(app->map.width * app->map.height * sizeof(char))	//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_Map == NULL)
		return false;
	#pragma endregion

	#pragma region TossRequestBuffer
	app->shmHandles.hSHM_TossReqBuffer = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,		//Desired access flag
		FALSE,						//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_TossRequestBuffer	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_TossRequestBuffer, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_TossReqBuffer == NULL)
		return false;

	app->shmHandles.lpSHM_TossReqBuffer = MapViewOfFile(
		app->shmHandles.hSHM_TossReqBuffer,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,				//Desired access flag
		0,							//DWORD high-order of the file offset where the view begins
		0,							//DWORD low-order of the file offset where the view begins
		sizeof(TossRequestsBuffer)	//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_TossRequestBuffer, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_TossReqBuffer == NULL)
		return false;
	#pragma endregion

	return true;
}

bool Setup_OpenThreadHandles(Application* app){
	TParam_NotificationReceiver_NT* nrParam = (TParam_NotificationReceiver_NT*) malloc(sizeof(TParam_NotificationReceiver_NT));
	nrParam->app = app;

	app->threadHandles.hNotificationReceiver_NewTransport = CreateThread(
		NULL,														//Security Attributes
		0,															//Stack Size (0 = default)
		Thread_NotificationReceiver_NewTransport,					//Function
		(LPVOID) nrParam,												//Param
		CREATE_SUSPENDED,											//Creation Flag
		&app->threadHandles.dwIdNotificationReceiver_NewTransport	//Thread ID
	);

	if(app->threadHandles.hNotificationReceiver_NewTransport == NULL)
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
	CloseHandle(syncHandles->hMutex_TossRequest_CanAccess);
	CloseHandle(syncHandles->hSemaphore_HasTossRequest);
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
	#pragma region Map
	//Is closed on Service_GetMap()
	#pragma endregion
	#pragma region TossRequestBuffer
	UnmapViewOfFile(shmHandles->lpSHM_TossReqBuffer);
	CloseHandle(shmHandles->hSHM_TossReqBuffer);
	#pragma endregion
}

void Setup_CloseThreadHandles(ThreadHandles* threadHandles){
	CloseHandle(threadHandles->hQnARequests);
	CloseHandle(threadHandles->hNotificationReceiver_NewTransport);
}

bool Service_GetMap(Application* app){
	if(app->shmHandles.hSHM_Map == NULL || app->shmHandles.lpSHM_Map == NULL){
		_tprintf(TEXT("%sThis function is not supposed to be running or something wrong happened!"), Utils_NewSubLine());
		return false;
	}

	app->map.cellArray = calloc(app->map.width * app->map.height, sizeof(char));
	CopyMemory(app->map.cellArray, app->shmHandles.lpSHM_Map, (app->map.width * app->map.height) * sizeof(char));

	//Since it only gets the map when login, there is no need to keep the map view of file open
	UnmapViewOfFile(app->shmHandles.lpSHM_Map);
	CloseHandle(app->shmHandles.hSHM_Map);
	app->shmHandles.lpSHM_Map = NULL;
	app->shmHandles.hSHM_Map = NULL;

	return true;
}