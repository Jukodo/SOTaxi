#pragma once
#include "Service.h"
#include "Communication.h"

bool isLoggedIn(Application* app){
	return !app->loggedInTaxi.deleted;
}

bool isValid_LicensePlate(TCHAR* sLicensePlate){
	if(wcslen(sLicensePlate) != 8)
		return false;

	for(unsigned int i = 0; i < wcslen(sLicensePlate); i++){
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
	if(wcslen(sCoordinates) <= 0 || wcslen(sCoordinates) > 2)
		return false;

	for(unsigned int i = 0; i < wcslen(sCoordinates); i++){
		if(!iswdigit(sCoordinates[i]))
			return false;
	}

	return true;
}

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->loggedInTaxi.deleted = true;

	return (Setup_OpenSyncHandles(&app->syncHandles) &&
		Setup_OpenSmhHandles(&app->shmHandles));
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->a = 1;
	syncHandles->mutex_SendRequest = OpenMutex(MUTEX_ALL_ACCESS, FALSE, NAME_MUTEX_RequestsTaxiToCentral);
	_tprintf(TEXT("\n%d"), &syncHandles->mutex_SendRequest);
	return true;
}
bool Setup_OpenSmhHandles(ShmHandles* shmHandles){
	shmHandles->hTestMem = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		SHM_Testing);

	if(shmHandles->hTestMem == NULL)
		return false;

	shmHandles->lpTestMem = MapViewOfFile(
		shmHandles->hTestMem,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(Taxi));

	if(shmHandles->lpTestMem == NULL)
		return false;

	return true;
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseSyncHandles(&app->syncHandles);
	Setup_CloseSmhHandles(&app->shmHandles);
}

void Setup_CloseSyncHandles(SyncHandles* syncHandles){
	CloseHandle(syncHandles->mutex_SendRequest);
}

void Setup_CloseSmhHandles(ShmHandles* shmHandles){
	#pragma region Testing_SHM
	UnmapViewOfFile(shmHandles->lpTestMem);
	CloseHandle(shmHandles->hTestMem);
	#pragma endregion
}

//void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){
//	_tprintf(TEXT("\n%d"), &app->syncHandles.mutex_SendRequest);
//	SendRequest request;
//	LoginRequest loginRequest;
//	_tcscpy_s(loginRequest.licensePlate, STRING_SMALL, sLicensePlate);
//	loginRequest.coordX = _wtoi(sCoordinates_X);
//	loginRequest.coordY = _wtoi(sCoordinates_Y);
//	request.app = app;
//	_tprintf(TEXT("\n%d"), &request.app->syncHandles.mutex_SendRequest);
//	request.loginRequest = loginRequest;
//	request.requestType = LOGIN;
//
//	app->threadHandles.hSendRequests = CreateThread(
//		NULL,
//		0,
//		Thread_SendRequests,				//Function
//		(LPVOID) &request,					//Param
//		0,									//Creation Flag
//		&app->threadHandles.dwIdSendRequests //idThread
//	);
//}