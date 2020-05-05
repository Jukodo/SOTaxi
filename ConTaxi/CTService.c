#pragma once
#include "CTService.h"
#include "CTCommunication.h"

bool isLoggedIn(Application* app){
	return !app->loggedInTaxi.empty;
}

bool isValid_LicensePlate(TCHAR* sLicensePlate){
	if(_tcslen(sLicensePlate) != 8)
		return false;

	for(unsigned int i = 0; i < _tcslen(sLicensePlate); i++){
		if((i+1)%3 == 0){ //every third character it is supposed to be a '-'
			if(sLicensePlate[i] == '-')
				continue;
			else
				return false;
		}

		if(!iswalpha(sLicensePlate[i]) && !iswdigit(sLicensePlate[i]))
			return false;
	}

	return true;
}

bool isValid_Coordinates(TCHAR* sCoordinates){
	if(_tcslen(sCoordinates) <= 0 || _tcslen(sCoordinates) > 2)
		return false;

	for(unsigned int i = 0; i < _tcslen(sCoordinates); i++){
		if(!iswdigit(sCoordinates[i]))
			return false;
	}

	return true;
}

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->loggedInTaxi.empty = true;

	return (Setup_OpenSyncHandles(&app->syncHandles) &&
		Setup_OpenSmhHandles(&app->shmHandles));
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hEvent_LARequest = CreateEvent(NULL, FALSE, TRUE, NAME_EVENT_LARequest);
	syncHandles->hEvent_LARequest_Read = CreateEvent(NULL, FALSE, FALSE, NAME_EVENT_LARequest_Read);
	syncHandles->hEvent_LARequest_Write = CreateEvent(NULL, FALSE, TRUE, NAME_EVENT_LARequest_Write);

	return !(syncHandles->hEvent_LARequest == NULL ||
		syncHandles->hEvent_LARequest_Read == NULL ||
		syncHandles->hEvent_LARequest_Write == NULL);
}
bool Setup_OpenSmhHandles(ShmHandles* shmHandles){
	shmHandles->hSHM_LARequest = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
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
	CloseHandle(syncHandles->hEvent_LARequest);
	CloseHandle(syncHandles->hEvent_LARequest_Read);
	CloseHandle(syncHandles->hEvent_LARequest_Write);
}

void Setup_CloseSmhHandles(ShmHandles* shmHandles){
#pragma region SendRequest
	UnmapViewOfFile(shmHandles->lpSHM_LARequest);
	CloseHandle(shmHandles->hSHM_LARequest);
#pragma endregion
}

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){
	TParam_LARequest* request = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));
	
	LoginRequest loginRequest;
	_tcscpy_s(loginRequest.licensePlate, _countof(loginRequest.licensePlate), sLicensePlate);
	loginRequest.coordX = _wtoi(sCoordinates_X);
	loginRequest.coordY = _wtoi(sCoordinates_Y);

	request->app = app;
	request->request.loginRequest = loginRequest;
	request->request.requestType = LOGIN;

	app->threadHandles.hLARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendLARequests,				//Function
		(LPVOID) request,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdLARequests  //Thread ID
	);
}