#pragma once
#include "CenService.h"
#include "CenCommunication.h"

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers){
	ZeroMemory(app, sizeof(Application));
	srand(time(NULL));
	app->settings.secAssignmentTimeout = DEFAULT_ASSIGNMENT_TIMEOUT;
	app->settings.allowTaxiLogins = DEFAULT_ALLOW_TAXI_LOGINS;

	app->maxTaxis = maxTaxis;
	app->maxPassengers = maxPassengers;

	app->taxiList = malloc(maxTaxis * sizeof(Taxi));
	app->passengerList = malloc(maxPassengers * sizeof(CenPassenger));
	
	int i;
	for(i = 0; i < maxTaxis; i++){
		ZeroMemory(&app->taxiList[i], sizeof(Taxi));
		app->taxiList[i].empty = true;
	}

	for(i = 0; i < maxPassengers; i++){
		ZeroMemory(&app->passengerList[i], sizeof(CenPassenger));
		app->passengerList[i].passengerInfo.empty = true;

		app->passengerList[i].interestedTaxis = malloc(maxTaxis * sizeof(int));
		for(int a = 0; a < maxTaxis; a++){
			app->passengerList[i].interestedTaxis[a] = -1;
		}
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
	TParam_QnARequest* param = (TParam_QnARequest*) malloc(sizeof(TParam_QnARequest));

	param->app = app;
	app->threadHandles.hQnARequests = CreateThread(
			NULL,								//Security Attributes
			0,									//Stack Size (0 = default)
			Thread_ReceiveQnARequests,			//Function
			(LPVOID) param,						//Param
			0,									//Creation flags
			&app->threadHandles.dwIdQnARequests //Thread Id
	);
	
	return !(app->threadHandles.hQnARequests == NULL);
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hEvent_QnARequest_Read = CreateEvent(
		NULL,						//Security Attributes
		FALSE,						//Manual Reset
		FALSE,						//Initial State
		NAME_EVENT_QnARequest_Read	//Event Name
	);
	syncHandles->hEvent_QnARequest_Write = CreateEvent(
		NULL,						//Security Attributes
		FALSE,						//Manual Reset
		TRUE,						//Initial State
		NAME_EVENT_QnARequest_Write	//Event Name
	);

	syncHandles->hEvent_Notify_T_NewTranspReq = CreateEvent(
		NULL,							//Security Attributes
		TRUE,							//Manual Reset
		FALSE,							//Initial State
		NAME_EVENT_NewTransportRequest	//Event Name
	);

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
		NAME_SHM_QnARequest		//File mapping object name
	);
	if(app->shmHandles.hSHM_QnARequest == NULL)
		return false;
	app->shmHandles.lpSHM_QnARequest = MapViewOfFile(
		app->shmHandles.hSHM_QnARequest,//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(QnARequest)				//Number of bytes to map
	);
	if(app->shmHandles.lpSHM_QnARequest == NULL)
		return false;
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
	#pragma region QnARequest
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

int Get_TaxiIndex(Application* app, TCHAR* licensePlate){
	if(app->taxiList == NULL)
		return -1;

	for(int i = 0; i < app->maxTaxis; i++){
		if(_tcscmp(app->taxiList[i].LicensePlate, licensePlate) == 0 && !app->taxiList[i].empty)
			return i;
	}

	return -1;
}

Taxi* Get_Taxi(Application* app, int index){
	if(app->taxiList == NULL)
		return NULL;

	if(!app->taxiList[index].empty)
		return &app->taxiList[index];

	return NULL;
}

int Get_QuantLoggedInPassengers(Application* app){
	int quantLoggedInPassengers = 0;

	for(int i = 0; i < app->maxPassengers; i++){
		if(!app->passengerList[i].passengerInfo.empty)
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
		if(app->passengerList[i].passengerInfo.empty)
			return i;
	}

	return -1;
}

int Get_PassengerIndex(Application* app, TCHAR* Id){
	if(app->passengerList == NULL)
		return -1;

	for(int i = 0; i < app->maxPassengers; i++){
		if(_tcscmp(app->passengerList[i].passengerInfo.Id, Id) == 0 && !app->passengerList[i].passengerInfo.empty)
			return i;
	}

	return -1;
}

CenPassenger* Get_Passenger(Application* app, int index){
	if(app->passengerList == NULL)
		return NULL;

	if(!app->passengerList[index].passengerInfo.empty)
		return &app->passengerList[index];

	return NULL;
}

bool isValid_ObjectPosition(Application* app, float coordX, float coordY){
	//ToDo

	return true;
}

bool Service_NewPassenger(Application* app, Passenger pass){
	if(isPassengerListFull(app))
		return false;

	int freeIndex = Get_FreeIndexPassengerList(app);
	if(freeIndex == -1)
		return false;

	app->passengerList[freeIndex].passengerInfo = pass;

	TParam_TaxiAssignment* param = (TParam_TaxiAssignment*) malloc(sizeof(TParam_TaxiAssignment));
	param->app = app;
	param->myIndex = freeIndex;
	app->passengerList[freeIndex].cpThreadHandles.hTaxiAssignment = CreateThread(//Each passenger will 
		NULL,															 //Security Attributes
		0,																 //Stack Size (0 = default)
		Thread_TaxiAssignment,											 //Function
		(LPVOID) param,													 //Param
		0,																 //Creation flags
		&app->passengerList[freeIndex].cpThreadHandles.dwIdTaxiAssignment//Thread Id
	);

	NewTransportBuffer* transportBuffer = (NewTransportBuffer*) app->shmHandles.lpSHM_NTBuffer;
	transportBuffer->transportRequests[transportBuffer->head] = pass;
	transportBuffer->head = (transportBuffer->head + 1) % NTBUFFER_MAX;
	Service_NotifyTaxisNewTransport(app);

	return true;
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

CentralCommands Service_UseCommand(Application* app, TCHAR* command){
	if(_tcscmp(command, CMD_HELP) == 0){ //Continues on Main (listing commands)
		return CC_HELP;
	} else if(_tcscmp(command, CMD_LIST_TAXIS) == 0){ //Continues on Main (listing logged in taxis)
		return CC_LIST_TAXIS;
	} else if(_tcscmp(command, CMD_SET_TIMEOUT) == 0){ //Continues on Main (asking for a value)
		return CC_SET_TIMEOUT;
	} else if(_tcscmp(command, CMD_TAXI_LOGIN_ON) == 0){
		Command_AllowTaxiLogins(app, true);
		return CC_TAXI_LOGIN_ON;
	} else if(_tcscmp(command, CMD_TAXI_LOGIN_OFF) == 0){
		Command_AllowTaxiLogins(app, false);
		return CC_TAXI_LOGIN_OFF;
	} else if(_tcscmp(command, CMD_KICK_TAXI) == 0){ //Continues on Main (asking for value argument)
		return CC_KICK_TAXI;
	} else if(_tcscmp(command, CMD_SIMULATE_NTR) == 0){ //Continues on Main (asking for value argument)
		Simulate_NewTransport(app);
		return CC_SIMULATE_NTR;
	} else if(_tcscmp(command, CMD_CLOSEAPP) == 0){
		Service_CloseApp(app);
		return CC_CLOSEAPP;
	}

	return CC_UNDEFINED;
}

LoginResponse Service_LoginTaxi(Application* app, LoginRequest* loginRequest){
	if(loginRequest == NULL || Utils_StringIsEmpty(loginRequest->licensePlate))
		return LR_INVALID_UNDEFINED;

	if(!app->settings.allowTaxiLogins)
		return LR_INVALID_CLOSED;

	if(isTaxiListFull(app))
		return LR_INVALID_FULL;

	if(!isValid_ObjectPosition(app, loginRequest->coordX, loginRequest->coordY))
		return LR_INVALID_POSITION;

	if(Get_TaxiIndex(app, loginRequest->licensePlate) != -1)
		return LR_INVALID_EXISTS;

	Taxi* anchorTaxi = &app->taxiList[Get_FreeIndexTaxiList(app)];
	anchorTaxi->empty = false;
	_tcscpy_s(anchorTaxi->LicensePlate, _countof(anchorTaxi->LicensePlate), loginRequest->licensePlate);
	anchorTaxi->object.coordX = loginRequest->coordX;
	anchorTaxi->object.coordY = loginRequest->coordY;

	return LR_SUCCESS;
}

NTInterestResponse Service_RegisterInterest(Application* app, NTInterestRequest* ntiRequest){
	if(ntiRequest == NULL || Utils_StringIsEmpty(ntiRequest->idPassenger))
		return NTIR_INVALID_UNDEFINED;

	int passIndex = Get_PassengerIndex(app, ntiRequest->idPassenger);
	int taxiIndex = Get_TaxiIndex(app, ntiRequest->licensePlate);

	if(taxiIndex == -1)//Taxi received is invalid (not supposed to ever happen!)
		return NTIR_INVALID_UNDEFINED;

	if(passIndex == -1)//Passenger doesn't exist
		return NTIR_INVALID_ID;

	if(WaitForSingleObject(app->passengerList[passIndex].cpThreadHandles.hTaxiAssignment, 0) == WAIT_OBJECT_0)
		return NTIR_INVALID_CLOSED;
	
	for(int i = 0; i < app->maxTaxis; i++){
		if(app->passengerList[passIndex].interestedTaxis[i] == -1){
			app->passengerList[passIndex].interestedTaxis[i] = taxiIndex;
			break;
		}
	}
	return NTIR_SUCCESS;
}

bool Service_KickTaxi(Application* app, TCHAR* licensePlate){
	return false;
}

void Service_CloseApp(Application* app){
	/*ToDo (TAG_TODO)
	**Notify taxis and passengers about this shutdown, in order to close everything accordingly
	*/
}

bool Command_SetAssignmentTimeout(Application* app, TCHAR* value){
	if(!Utils_StringIsNumber(value))
		return false;

	int timeoutValue = _ttoi(value);
	if(timeoutValue <= 0 || timeoutValue > TOPMAX_ASSIGNMENT_TIMEOUT){
		_tprintf(TEXT("%sAssignment timeout value has to be between 1 and %d! Hence, it remained the same (%d)..."), Utils_NewSubLine(), TOPMAX_ASSIGNMENT_TIMEOUT, app->settings.secAssignmentTimeout);
		return true;
	}

	app->settings.secAssignmentTimeout = timeoutValue;
	_tprintf(TEXT("%sAssignment timeout value has been changed to %d!"), Utils_NewSubLine(), app->settings.secAssignmentTimeout);
	return true;
}

void Command_AllowTaxiLogins(Application* app, bool allow){
	app->settings.allowTaxiLogins = allow;

	if(app->settings.allowTaxiLogins)
		_tprintf(TEXT("%sTaxi logins are now allowed!"), Utils_NewSubLine());
	else
		_tprintf(TEXT("%sTaxi logins are now being denied!"), Utils_NewSubLine());
}

void Simulate_NewTransport(Application* app){
	Passenger tempP;
	tempP.empty = false;
	tempP.object.coordX = 1;
	tempP.object.coordY = 1;

	_stprintf_s(tempP.Id, _countof(tempP.Id), TEXT("Pass%d"), rand() % 50 + 10);
	if(!Service_NewPassenger(app, tempP))
		_tprintf(TEXT("%sPassenger limit has been reached... This passenger will be ignored!"), Utils_NewSubLine());
}