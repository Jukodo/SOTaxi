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

	syncHandles->hEvent_PassengerList_Access = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_PassengerList_Access	//Event name
	);

	syncHandles->hEvent_Notify_T_NP = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_Notify_T_NP			//Event name
	);

	return !(syncHandles->hMutex_LARequest == NULL ||
		syncHandles->hEvent_LARequest_Read == NULL ||
		syncHandles->hEvent_LARequest_Write == NULL ||
		syncHandles->hEvent_PassengerList_Access == NULL ||
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

	Service_RequestVars(app);
	WaitForSingleObject(app->threadHandles.hLARequests, INFINITE);

	if(app->maxTaxis <= 0 || app->maxTaxis > TOPMAX_TAXI ||
		app->maxPassengers <= 0 || app->maxPassengers > TOPMAX_PASSENGERS)
		return false;

	#pragma region PassengerList
	app->shmHandles.hSHM_PassengerList = OpenFileMapping(
		FILE_MAP_READ,			//Desired access flag
		FALSE,					//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_PASSLIST		//File mapping object name
	);
	if(app->shmHandles.hSHM_PassengerList == NULL)
		return false;

	app->shmHandles.lpSHM_PassengerList = MapViewOfFile(
		app->shmHandles.hSHM_PassengerList,		//File mapping object handle
		FILE_MAP_READ,							//Desired access flag
		0,										//DWORD high-order of the file offset where the view begins
		0,										//DWORD low-order of the file offset where the view begins
		sizeof(Passenger) * app->maxPassengers	//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_PassengerList == NULL)
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

void Setup_CloseThreadHandles(ThreadHandles* threadHandles){
	CloseHandle(threadHandles->hLARequests);
	CloseHandle(threadHandles->hNotificationReceiver_NP);
}

void Service_RequestVars(Application* app){
	TParam_LARequest* request = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));

	request->app = app;
	request->request.requestType = RT_VAR;

	app->threadHandles.hLARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendLARequests,				//Function
		(LPVOID) request,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdLARequests  //Thread ID
	);
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
	TParam_LARequest* param = (TParam_LARequest*) malloc(sizeof(TParam_LARequest));

	AssignRequest assignRequest;
	_tcscpy_s(assignRequest.idPassenger, _countof(assignRequest.idPassenger), idPassenger);

	param->app = app;
	param->request.assignRequest = assignRequest;
	param->request.requestType = RT_ASSIGN;

	app->threadHandles.hLARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendLARequests,				//Function
		(LPVOID) param,						//Param
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
	TParam_ListPassengers* param = (TParam_ListPassengers*) malloc(sizeof(TParam_ListPassengers));
	param->app = app;

	app->threadHandles.hLARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_ListPassenger,				//Function
		(LPVOID) param,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdLARequests  //Thread ID
	);
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