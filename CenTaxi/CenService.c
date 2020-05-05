#pragma once
#include "CenService.h"
#include "CenCommunication.h"

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers){
	ZeroMemory(app, sizeof(Application));

	app->taxiList = malloc(maxTaxis * sizeof(Taxi));
	app->passengerList = malloc(maxPassengers * sizeof(Passenger));

	app->maxTaxis = maxTaxis;
	app->maxPassengers = maxPassengers;

	int i;
	if(app->taxiList != NULL){
		for(i = 0; i < maxTaxis; i++){
			ZeroMemory(&app->taxiList[i], sizeof(Taxi));
			app->taxiList[i].empty = true;
		}
	}
	if(app->passengerList != NULL){
		for(i = 0; i < maxTaxis; i++){
			ZeroMemory(&app->passengerList[i], sizeof(Passenger));
			app->passengerList[i].empty = true;
		}
	}

	bool ret = true;
	ret = ret && (app->taxiList != NULL);
	ret = ret && (app->passengerList != NULL);
	ret = ret && Setup_OpenSyncHandles(&app->syncHandles);
	ret = ret && Setup_OpenSmhHandles(&app->shmHandles);
	ret = ret && Setup_OpenThreadHandles(app); //Has to be called at the end, because it will use Sync and SMH

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

int Get_QuantLoggedInTaxis(Application* app){
	int quantLoggedInTaxis = 0;

	for(int i = 0; i < app->maxTaxis; i++){
		if(!app->taxiList[i].empty)
			quantLoggedInTaxis++;
	}

	return quantLoggedInTaxis;
}

bool isTaxiListFull(Application* app){
	return Get_QuantLoggedInTaxis(app) >= app->maxTaxis;
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

	for(int i = 0; i < app->maxTaxis; i++){
		if(!app->taxiList[i].empty)
			quantLoggedInPassengers++;
	}

	return quantLoggedInPassengers;
}

bool isPassengerListFull(Application* app){
	return Get_QuantLoggedInPassengers(app) >= app->maxTaxis;
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
	if(app->taxiList == NULL)
		return NULL;

	for(int i = 0; i < app->maxPassengers; i++){
		if(_tcscmp(app->passengerList[i].Id, Id) == 0)
			return &app->passengerList[i];
	}

	return NULL;
}

bool isValid_ObjectPosition(Application* app, int coordX, int coordY){
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