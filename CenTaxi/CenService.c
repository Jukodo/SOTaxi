#pragma once
#include "CenService.h"
#include "CenCommunication.h"

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers){
	ZeroMemory(app, sizeof(Application));

	app->maxTaxis = maxTaxis;
	app->maxPassengers = maxPassengers;

	app->taxiList = malloc(maxTaxis * sizeof(Taxi));
	app->passengerList = malloc(maxPassengers * sizeof(Passenger));
	
	int i;
	for(i = 0; i < maxTaxis; i++){
		ZeroMemory(&app->taxiList[i], sizeof(Taxi));
		app->taxiList[i].empty = true;
	}

	for(i = 0; i < maxPassengers; i++){
		ZeroMemory(&app->passengerList[i], sizeof(Passenger));
		app->passengerList[i].empty = true;
	}

	bool ret = true;
	ret = ret && (app->passengerList != NULL);
	ret = ret && (app->taxiList != NULL);
	ret = ret && Setup_OpenSyncHandles(&app->syncHandles);
	ret = ret && Setup_OpenSmhHandles(app);
	ret = ret && Setup_OpenThreadHandles(app); //Has to be called at the end, because it will use Sync and SMH

	return ret;
}

bool Setup_OpenThreadHandles(Application* app){
	TParam_LARequest* param = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));

	param->app = app;
	app->threadHandles.hQnARequests = CreateThread(
			NULL,								//Security Attributes
			0,									//Stack Size (0 = default)
			Thread_ReceiveQnARequests,			//Function
			(LPVOID) param,						//Param
			0,									//Creation flags
			&app->threadHandles.dwIdQnARequests  //Thread Id
	);
	
	return !(app->threadHandles.hQnARequests == NULL);
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hEvent_QnARequest_Read = CreateEvent(NULL, FALSE, FALSE, NAME_EVENT_QnARequest_Read);
	syncHandles->hEvent_QnARequest_Write = CreateEvent(NULL, FALSE, TRUE, NAME_EVENT_QnARequest_Write);

	syncHandles->hEvent_Notify_T_NewTranspReq = CreateEvent(NULL, TRUE, FALSE, NAME_EVENT_NewTransportRequest);

	return !(syncHandles->hEvent_QnARequest_Read == NULL ||
		syncHandles->hEvent_QnARequest_Write == NULL ||
		syncHandles->hEvent_Notify_T_NewTranspReq == NULL);
}

bool Setup_OpenSmhHandles(Application* app){
	#pragma region LARequest
	app->shmHandles.hSHM_QnARequest = CreateFileMapping(
		INVALID_HANDLE_VALUE,	//File handle
		NULL,					//Security Attributes
		PAGE_READWRITE,			//Protection flags
		0,						//DWORD high-order max size
		sizeof(QnARequest),		//DWORD low-order max size
		NAME_SHM_QnARequest	//File mapping object name
	);
	if(app->shmHandles.hSHM_QnARequest == NULL)
		return false;
	app->shmHandles.lpSHM_QnARequest = MapViewOfFile(
		app->shmHandles.hSHM_QnARequest, //File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(QnARequest)				//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_QnARequest == NULL)
		return false;
	_tprintf(TEXT("lpSHM_QnARequest is not null"));
	#pragma endregion

	#pragma region NewTransportBuffer
	app->shmHandles.hSHM_NTBuffer = CreateFileMapping(
		INVALID_HANDLE_VALUE,			//File handle
		NULL,							//Security Attributes
		PAGE_READWRITE,					//Protection flags
		0,								//DWORD high-order max size
		sizeof(NewTransportBuffer),		//DWORD low-order max size
		NAME_SHM_TransportRequestBuffer	//File mapping object name
	);
	if(app->shmHandles.hSHM_NTBuffer == NULL)
		return false;

	app->shmHandles.lpSHM_NTBuffer = MapViewOfFile(
		app->shmHandles.hSHM_NTBuffer,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(NewTransportBuffer)		//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_NTBuffer == NULL)
		return false;

	ZeroMemory(app->shmHandles.lpSHM_NTBuffer, sizeof(NewTransportBuffer)); //Makes sure head starts at 0 (unecessary to zero everything)
	#pragma endregion

	return true;
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseSyncHandles(&app->syncHandles);
	Setup_CloseSmhHandles(&app->shmHandles);
}

void Setup_CloseSyncHandles(SyncHandles* syncHandles){
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

	int freeIndex = Get_FreeIndexPassengerList(app);
	if(freeIndex == -1)
		return false;

	_tprintf(TEXT("%d"), freeIndex);

	app->passengerList[freeIndex] = pass;
	NewTransportBuffer* transportBuffer = (NewTransportBuffer*) app->shmHandles.lpSHM_NTBuffer;
	transportBuffer->transportRequests[transportBuffer->head] = pass;
	transportBuffer->head = (transportBuffer->head + 1) % NTBUFFER_MAX;
	Service_NotifyTaxisNewTransport(app);

	return true;
}

AssignResponse Service_RequestPassenger(Application* app, AssignRequest* assignRequest){
	return AR_SUCCESS;
}

void Service_NotifyTaxisNewTransport(Application* app){
	//Manual reset event
	SetEvent(app->syncHandles.hEvent_Notify_T_NewTranspReq); 
	ResetEvent(app->syncHandles.hEvent_Notify_T_NewTranspReq);
	//OR
	//Auto reset event (tested, and no flaws found)
	/*for(int i = 0; i < Get_QuantLoggedInTaxis(app); i++)
		SetEvent(app->syncHandles.hEvent_Notify_T_NP);*/
}