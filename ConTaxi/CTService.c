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
	syncHandles->hMutex_LARequest = CreateMutex(NULL, FALSE, NAME_MUTEX_LARequest);
	syncHandles->hEvent_LARequest_Read = CreateEvent(NULL, FALSE, FALSE, NAME_EVENT_LARequest_Read);
	syncHandles->hEvent_LARequest_Write = CreateEvent(NULL, FALSE, TRUE, NAME_EVENT_LARequest_Write);

	return !(syncHandles->hMutex_LARequest == NULL ||
		syncHandles->hEvent_LARequest_Read == NULL ||
		syncHandles->hEvent_LARequest_Write == NULL);
}
bool Setup_OpenSmhHandles(ShmHandles* shmHandles){
	shmHandles->hSHM_LARequest = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		NAME_SHM_LAREQUESTS);

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

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){
	TParam_LARequest* request = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));
	
	LoginRequest loginRequest;
	_tcscpy_s(loginRequest.licensePlate, _countof(loginRequest.licensePlate), sLicensePlate);
	loginRequest.coordX = (float) _tstof(sCoordinates_X);
	loginRequest.coordY = (float) _tstof(sCoordinates_Y);

	request->app = app;
	request->request.loginRequest = loginRequest;
	request->request.requestType = RT_LOGIN;

	app->threadHandles.hLARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendLARequests,				//Function
		(LPVOID) request,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdLARequests  //Thread ID
	);
}

TaxiCommands Service_UseCommand(Application* app, TCHAR* command){
	if(_tcscmp(command, CMD_HELP) == 0){ //Continues on Main (listing commands)
		return TC_HELP;
	} else if(_tcscmp(command, CMD_SPEED_UP) == 0){
		Command_Speed(app, true);
		return TC_SPEED_UP;
	} else if(_tcscmp(command, CMD_SPEED_DOWN) == 0){
		Command_Speed(app, false);
		return TC_SPEED_DOWN;
	} else if(_tcscmp(command, CMD_AUTORESP_ON) == 0){
		Command_AutoResp(app, true);
		return TC_AUTORESP_ON;
	} else if(_tcscmp(command, CMD_AUTORESP_OFF) == 0){
		Command_AutoResp(app, false);
		return TC_AUTORESP_OFF;
	} else if(_tcscmp(command, CMD_LISTPASS) == 0){
		Command_ListPassengers(app);
		return TC_LISTPASS;
	} else if(_tcscmp(command, CMD_DEFINE_CDN) == 0){ //Continues on Main (asking for value argument)
		return TC_DEFINE_CDN;
	} else if(_tcscmp(command, CMD_REQUEST_PASS) == 0){ //Continues on Main (asking for value argument)
		return TC_REQUEST_PASS;
	} else if(_tcscmp(command, CMD_CLOSEAPP) == 0){
		Command_CloseApp(app);
		return TC_CLOSEAPP;
	}

	return TC_UNDEFINED;
}

void Service_RequestPass(Application* app, TCHAR* idPassenger){
	TParam_LARequest* request = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));

	AssignRequest assignRequest;
	_tcscpy_s(assignRequest.idPassenger, _countof(assignRequest.idPassenger), idPassenger);

	request->app = app;
	request->request.assignRequest = assignRequest;
	request->request.requestType = RT_ASSIGN;

	app->threadHandles.hLARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendLARequests,				//Function
		(LPVOID) request,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdLARequests  //Thread ID
	);
}

bool Service_DefineCDN(Application* app, TCHAR* value){
	if(!Utils_StringIsNumber(value))
		return false;

	int cdnValue = _ttoi(value);
	//ToDo
	return true;
}

void Command_ListPassengers(Application* app){
	//ToDo
}

void Command_Speed(Application* app, bool speedUp){
	//ToDo
}

void Command_AutoResp(Application* app, bool autoResp){
	//ToDo
}

void Command_CloseApp(Application* app){
	//ToDo
}