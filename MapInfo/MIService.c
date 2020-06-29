#pragma once
#include "MIService.h"

bool Setup_Application(Application* app, HWND hWnd){
	ZeroMemory(app, sizeof(Application));
	srand((unsigned int) time(NULL));

	if(!Setup_OpenSyncHandles(&app->syncHandles)){
		return false;
	}
	if(!Setup_OpenShmHandles(app)){
		return false;
}
	if(!Setup_OpenThreadHandles(app)){
		return false;
	}

	app->refreshRoutine.mapRelativeHeight = (int) round(GetSystemMetrics(SM_CYSCREEN) * 0.8);
	app->refreshRoutine.mapRelativeWidth = app->refreshRoutine.mapRelativeHeight;

	app->refreshRoutine.xMapOffset = 25; //Let initial space on left side for coordinates indicators
	app->refreshRoutine.yMapOffset = 25; //Let initial space on top side for coordinates indicators

	RECT windowSize;
	if(GetClientRect(hWnd, &windowSize)){
		app->refreshRoutine.mapRelativeHeight = windowSize.bottom - windowSize.top - app->refreshRoutine.yMapOffset;
		app->refreshRoutine.mapRelativeWidth = app->refreshRoutine.mapRelativeHeight;
	}

	app->refreshRoutine.cellWidth = (app->refreshRoutine.mapRelativeWidth / app->map.width)+1;
	app->refreshRoutine.cellHeight = (app->refreshRoutine.mapRelativeHeight /app->map.height)+1;

	return true;
}

bool Setup_OpenThreadHandles(Application* app){
	return true;
}

bool Setup_OpenThreadHandles_RefreshRoutine(ThreadHandles* threadHandles, HANDLE hWnd){
	#pragma region Refresh Routine
	TParam_RefreshRoutine* rrParam = (TParam_RefreshRoutine*) malloc(sizeof(TParam_RefreshRoutine));
	rrParam->hWnd = hWnd;

	threadHandles->hRefreshRoutine = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_RefreshRoutine,				//Function
		(LPVOID) rrParam,					//Param
		0,									//Creation flags
		&threadHandles->dwIdRefreshRoutine	//Thread Id
	);
	#pragma endregion

	return !(threadHandles->hRefreshRoutine == NULL);
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hMutex_QnARequest_CanAccess = CreateMutex(//This mutex is only created on and for ConTaxi
		NULL,							//Security attributes
		FALSE,							//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_QnARequest);			//Mutex name
	Utils_DLL_Register(NAME_MUTEX_QnARequest, DLL_TYPE_MUTEX);

	syncHandles->hEvent_QnARequest_Read = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Read);	//Event name
	Utils_DLL_Register(NAME_EVENT_QnARequest_Read, DLL_TYPE_EVENT);

	syncHandles->hEvent_QnARequest_Write = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Write);	//Event name
	Utils_DLL_Register(NAME_EVENT_QnARequest_Write, DLL_TYPE_EVENT);

	return !(syncHandles->hMutex_QnARequest_CanAccess == NULL ||
		syncHandles->hEvent_QnARequest_Read == NULL ||
		syncHandles->hEvent_QnARequest_Write == NULL);
}
bool Setup_OpenShmHandles(Application* app){
	#pragma region QnARequest
	app->shmHandles.hSHM_QnARequest = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	//Desired access flag
		FALSE,					//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_QnARequest	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_QnARequest == NULL){
		return false;
	}

	app->shmHandles.lpSHM_QnARequest = MapViewOfFile(
		app->shmHandles.hSHM_QnARequest,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(QnARequest)				//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_QnARequest == NULL){
		return false;
}
	#pragma endregion

	//Now that QnARequest SHM is mapped, we can request vars (maxTaxis, maxPassengers, mapWidth and mapHeight) from CenTaxi to open the next SHM (taxiList, passList, map)
	if(!Service_RequestMaxVars(app)){
		return false;
	}

	#pragma region Map
	app->shmHandles.hSHM_Map = OpenFileMapping(
		FILE_MAP_READ,					//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_Map	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_Map == NULL){
		return false;
	}

	app->shmHandles.lpSHM_Map = MapViewOfFile(
		app->shmHandles.hSHM_Map,	//File mapping object handle
		FILE_MAP_READ,				//Desired access flag
		0,							//DWORD high-order of the file offset where the view begins
		0,							//DWORD low-order of the file offset where the view begins
		(app->map.width * app->map.height * sizeof(char))	//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_Map == NULL){
		return false;
	}

	//Since nothing went wrong from openning the map from CenTaxi, it creates a static array and closes map shared memory, since it will not be needed anymore
	app->map.cellArray = calloc(app->map.width * app->map.height, sizeof(char));
	CopyMemory(app->map.cellArray, app->shmHandles.lpSHM_Map, (app->map.width * app->map.height) * sizeof(char));
	UnmapViewOfFile(app->shmHandles.lpSHM_Map);
	CloseHandle(app->shmHandles.hSHM_Map);
	app->shmHandles.lpSHM_Map = NULL;
	app->shmHandles.hSHM_Map = NULL;
	#pragma endregion

	#pragma region TaxiList
	#pragma endregion

	#pragma region PassengerList
	#pragma endregion
	return true;
}

void Setup_CloseAllHandles(Application* app){
}
void Setup_CloseSyncHandles(SyncHandles* syncHandles){
}
void Setup_CloseShmHandles(ShmHandles* shmHandles){
}

bool Service_RequestMaxVars(Application* app){
	QnARequest* shm = app->shmHandles.lpSHM_QnARequest;
	if(shm == NULL){
		return false;
	}

	WaitForSingleObject(app->syncHandles.hMutex_QnARequest_CanAccess, INFINITE);
	WaitForSingleObject(app->syncHandles.hEvent_QnARequest_Write, INFINITE);

	QnARequest maxVarsRequest;
	maxVarsRequest.requestType = QnART_MAX_VARS;

	CopyMemory(shm, &maxVarsRequest, sizeof(QnARequest));
	SetEvent(app->syncHandles.hEvent_QnARequest_Read);

	WaitForSingleObject(app->syncHandles.hEvent_QnARequest_Write, INFINITE);

	app->maxTaxis = shm->maxVarsResponse.maxTaxis;
	app->maxPassengers = shm->maxVarsResponse.maxPassengers;
	app->map.width = shm->maxVarsResponse.mapWidth;
	app->map.height = shm->maxVarsResponse.mapHeight;

	ReleaseMutex(app->syncHandles.hMutex_QnARequest_CanAccess);
	SetEvent(app->syncHandles.hEvent_QnARequest_Write);

	if(app->maxTaxis < 0 ||
		app->maxPassengers < 0 ||
		app->map.width < 0 ||
		app->map.height < 0){
		return false;
	}

	return true;
}

bool Service_UpdateTaxiList(Application* app){

	return true;
}
bool Service_UpdatePassengerList(Application* app){

	return true;
}