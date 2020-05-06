#pragma once
#include "CenService.h"
#include "CenCommunication.h"

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers){
	ZeroMemory(app, sizeof(Application));

	app->maxTaxis = maxTaxis;
	app->maxPassengers = maxPassengers;

	bool ret = true;
	ret = ret && Setup_OpenSyncHandles(&app->syncHandles);
	ret = ret && Setup_OpenSmhHandles(app);
	ret = ret && Setup_OpenThreadHandles(app); //Has to be called at the end, because it will use Sync and SMH

	app->taxiList = malloc(maxTaxis * sizeof(Taxi));
	//Set SHM pointer into the application struct (in order to access taxiList and passengerList in the same place)
	app->passengerList = app->shmHandles.lpSHM_PassengerList;
	
	ret = ret && (app->passengerList != NULL);
	ret = ret && (app->taxiList != NULL);

	if(ret){
		int i;
		for(i = 0; i < maxTaxis; i++){
			ZeroMemory(&app->taxiList[i], sizeof(Taxi));
			app->taxiList[i].empty = true;
		}

		for(int i = 0; i < maxTaxis; i++){
			ZeroMemory(&app->passengerList[i], sizeof(Passenger));
			app->passengerList[i].empty = true;
		}
	}

	return ret;
}

bool Setup_OpenThreadHandles(Application* app){
	TParam_LARequest* param = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));

	param->app = app;
	app->threadHandles.hLARequests = CreateThread(
			NULL,								//Security Attributes
			0,									//Stack Size (0 = default)
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

	syncHandles->hEvent_PassengerList_Access = CreateEvent(NULL, FALSE, TRUE, NAME_EVENT_PassengerList_Access);

	return !(syncHandles->hEvent_LARequest_Read == NULL ||
		syncHandles->hEvent_LARequest_Write == NULL ||
		syncHandles->hEvent_PassengerList_Access == NULL);
}

bool Setup_OpenSmhHandles(Application* app){

	#pragma region LARequest
	app->shmHandles.hSHM_LARequest = CreateFileMapping(
		INVALID_HANDLE_VALUE,	//File handle
		NULL,					//Security Attributes
		PAGE_READWRITE,			//Protection flags
		0,						//DWORD high-order max size
		sizeof(LARequest),		//DWORD low-order max size
		NAME_SHM_LAREQUESTS		//File mapping object name
	);
	if(app->shmHandles.hSHM_LARequest == NULL)
		return false;

	app->shmHandles.lpSHM_LARequest = MapViewOfFile(
		app->shmHandles.hSHM_LARequest, //File mapping object handle
		FILE_MAP_ALL_ACCESS,		//Desired access flag
		0,							//DWORD high-order of the file offset where the view begins
		0,							//DWORD low-order of the file offset where the view begins
		sizeof(LARequest)			//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_LARequest == NULL)
		return false;
	#pragma endregion

	if(app->maxPassengers <= 0 || app->maxPassengers > TOPMAX_PASSENGERS)
		return false;

	#pragma region PassengerList
	app->shmHandles.hSHM_PassengerList = CreateFileMapping(
		INVALID_HANDLE_VALUE,					//File handle
		NULL,									//Security Attributes
		PAGE_READWRITE,							//Protection flags
		0,										//DWORD high-order max size
		sizeof(Passenger) * app->maxPassengers,	//DWORD low-order max size
		NAME_SHM_PASSLIST						//File mapping object name
	);
	if(app->shmHandles.hSHM_PassengerList == NULL)
		return false;

	app->shmHandles.lpSHM_PassengerList = MapViewOfFile(
		app->shmHandles.hSHM_PassengerList,		//File mapping object handle
		FILE_MAP_ALL_ACCESS,					//Desired access flag
		0,										//DWORD high-order of the file offset where the view begins
		0,										//DWORD low-order of the file offset where the view begins
		sizeof(Passenger) * app->maxPassengers	//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_PassengerList == NULL)
		return false;
	#pragma endregion

	return true;
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseSyncHandles(&app->syncHandles);
	Setup_CloseSmhHandles(&app->shmHandles);
}

void Setup_CloseSyncHandles(SyncHandles* syncHandles){
	CloseHandle(syncHandles->hEvent_LARequest_Read);
	CloseHandle(syncHandles->hEvent_LARequest_Write);
	CloseHandle(syncHandles->hEvent_PassengerList_Access);
}

void Setup_CloseSmhHandles(ShmHandles* shmHandles){
	#pragma region SendRequest
	UnmapViewOfFile(shmHandles->lpSHM_LARequest);
	CloseHandle(shmHandles->hSHM_LARequest);
	#pragma endregion
	#pragma region PassengerList
	UnmapViewOfFile(shmHandles->lpSHM_PassengerList);
	CloseHandle(shmHandles->hSHM_PassengerList);
	#pragma endregion
}

bool isTaxiListFull(Application* app){
	return Get_QuantLoggedInTaxis(app) >= app->maxTaxis;
}

int Get_QuantLoggedInTaxis(Application* app){
	int quantLoggedInTaxis = 0;

	for(int i = 0; i < app->maxTaxis; i++){
		if(!app->taxiList[i].empty)
			quantLoggedInTaxis++;
	}

	return quantLoggedInTaxis;
}

int Get_FreeIndexTaxiList(Application* app){
	if(isTaxiListFull(app))
		return -1;

	for(int i = 0; i < app->maxTaxis; i++){
		if(app->taxiList[i].empty)
			return i;
	}

	return -1;
}

Taxi* Get_Taxi(Application* app, TCHAR* licensePlate){
	if(app->taxiList == NULL)
		return NULL;

	for(int i = 0; i < app->maxTaxis; i++){
		if(_tcscmp(app->taxiList[i].LicensePlate, licensePlate) == 0)
			return &app->taxiList[i];
	}

	return NULL;
}

int Get_QuantLoggedInPassengers(Application* app){
	int quantLoggedInPassengers = 0;

	for(int i = 0; i < app->maxPassengers; i++){
		if(!app->passengerList[i].empty)
			quantLoggedInPassengers++;
	}

	return quantLoggedInPassengers;
}

bool isPassengerListFull(Application* app){
	return Get_QuantLoggedInPassengers(app) >= app->maxPassengers;
}

int Get_FreeIndexPassengerList(Application* app){
	if(isPassengerListFull(app))
		return -1;

	for(int i = 0; i < app->maxPassengers; i++){
		if(app->passengerList[i].empty)
			return i;
	}

	return -1;
}

Passenger* Get_Passenger(Application* app, TCHAR* Id){
	if(app->passengerList == NULL)
		return NULL;

	for(int i = 0; i < app->maxPassengers; i++){
		if(_tcscmp(app->passengerList[i].Id, Id) == 0)
			return &app->passengerList[i];
	}

	return NULL;
}

bool isValid_ObjectPosition(Application* app, float coordX, float coordY){
	//ToDo

	return true;
}

LoginResponse Service_LoginTaxi(Application* app, LoginRequest* loginRequest){
	if(loginRequest == NULL || Utils_StringIsEmpty(loginRequest->licensePlate))
		return LR_INVALID_UNDEFINED;

	if(isTaxiListFull(app))
		return LR_INVALID_FULL;

	if(!isValid_ObjectPosition(app, loginRequest->coordX, loginRequest->coordY))
		return LR_INVALID_POSITION;
	

	Taxi* anchorTaxi = &app->taxiList[Get_FreeIndexTaxiList(app)];
	anchorTaxi->empty = false;
	_tcscpy_s(anchorTaxi->LicensePlate, _countof(anchorTaxi->LicensePlate), loginRequest->licensePlate);
	anchorTaxi->object.coordX = loginRequest->coordX;
	anchorTaxi->object.coordY = loginRequest->coordY;
	
	return LR_SUCCESS;
}

bool Service_NewPassenger(Application* app, Passenger pass){
	if(isPassengerListFull(app))
		return false;

	WaitForSingleObject(app->syncHandles.hEvent_PassengerList_Access, INFINITE);

	int freeIndex = Get_FreeIndexPassengerList(app);
	if(freeIndex == -1)
		return false;

	app->passengerList[freeIndex] = pass;

	SetEvent(app->syncHandles.hEvent_PassengerList_Access);
	return true;
}

AssignResponse Service_RequestPassenger(Application* app, AssignRequest* assignRequest){
	return AR_SUCCESS;
}