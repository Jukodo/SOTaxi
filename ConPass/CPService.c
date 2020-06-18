#pragma once
#include "CPService.h"

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->keepRunning = true;

	bool ret = true;

	ret = ret && Setup_OpenNamedPipeHandles(&app->namedPipeHandles);
	if(!ret)
		return false;

	ret = ret && Setup_OpenThreadHandles(app);
	if(!ret)
		return false;

	ret = ret && Setup_OpenSyncHandles(&app->syncHandles);
	if(!ret)
		return false;

	app->maxPass = -1;
	Service_GetMaxPass(app);
	ret = ret && (app->maxPass > 0 && app->maxPass <= TOPMAX_PASSENGERS);
	if(!ret)
		return false;

	app->passengerList = calloc(app->maxPass, sizeof(Passenger));
	ret = ret && app->passengerList != NULL;
	if(!ret)
		return false;

	for(int i = 0; i < app->maxPass; i++)
		app->passengerList[i].passengerInfo.empty = true;

	return ret;
}

bool Setup_OpenNamedPipeHandles(NamedPipeHandles* namedPipeHandles){
	#pragma region ReadFromCentral
	HANDLE hPipe_Read = CreateFile(
		NAME_NAMEDPIPE_CommsC2P,	//Named Pipe name
		GENERIC_READ,				//Read Access
		0,							//Doesn't share
		NULL,						//Security attributes
		OPEN_EXISTING,				//Open existing pipe 
		0,							//Atributes
		NULL);						//Template file 

	if(hPipe_Read == INVALID_HANDLE_VALUE){
		if(GetLastError() == ERROR_PIPE_BUSY)
			_tprintf(TEXT("%sSomething unexpected happened! Seems like a ConPass is already connected to the central..."), Utils_NewSubLine());

		CloseHandle(hPipe_Read);
		return false;
	}

	namedPipeHandles->hRead = hPipe_Read;
	#pragma endregion

	#pragma region WriteToCentral
	HANDLE hPipe_Write = CreateFile(
		NAME_NAMEDPIPE_CommsP2C,	//Named Pipe name
		GENERIC_WRITE,				//Write Access
		0,							//Doesn't share
		NULL,						//Security attributes
		OPEN_EXISTING,				//Open existing pipe 
		0,							//Atributes
		NULL);						//Template file 

	if(hPipe_Write == INVALID_HANDLE_VALUE){
		if(GetLastError() == ERROR_PIPE_BUSY)
			_tprintf(TEXT("%sSomething unexpected happened! Seems like a ConPass is already connected to the central..."), Utils_NewSubLine());
		else
			_tprintf(TEXT("%sCreateFile failed! Error: %d"), Utils_NewSubLine(), GetLastError());

		CloseHandle(hPipe_Write);
		return false;
	}

	namedPipeHandles->hWrite = hPipe_Write;
	#pragma endregion

	#pragma region QnAToCentral
	HANDLE hPipe_QnA = CreateFile(
		NAME_NAMEDPIPE_CommsPCQnA,	//Named Pipe name
		GENERIC_READ |				//Read Access
		GENERIC_WRITE,				//Write Access
		0,							//Doesn't share
		NULL,						//Security attributes
		OPEN_EXISTING,				//Open existing pipe 
		0,							//Atributes
		NULL);						//Template file 

	if(hPipe_QnA == INVALID_HANDLE_VALUE){
		if(GetLastError() == ERROR_PIPE_BUSY)
			_tprintf(TEXT("%sSomething unexpected happened! Seems like a ConPass is already connected to the central..."), Utils_NewSubLine());
		else
			_tprintf(TEXT("%sCreateFile failed! Error: %d"), Utils_NewSubLine(), GetLastError());

		CloseHandle(hPipe_QnA);
		return false;
	}

	namedPipeHandles->hQnA = hPipe_QnA;
	#pragma endregion

	return true;
}

bool Setup_OpenThreadHandles(Application* app){
	#pragma region Notification Receiver from Central Named Pipe
	TParam_NotificationReceiver_NamedPipe* nrnpParam = (TParam_NotificationReceiver_NamedPipe*) malloc(sizeof(TParam_NotificationReceiver_NamedPipe));
	nrnpParam->app = app;

	app->threadHandles.hNotificationReceiver_NamedPipe = CreateThread(
		NULL,													//Security Attributes
		0,														//Stack Size (0 = default)
		Thread_NotificationReceiver_NamedPipe,					//Function
		(LPVOID) nrnpParam,										//Param
		0,														//Creation flags
		&app->threadHandles.dwIdNotificationReceiver_NamedPipe	//Thread Id
	);
	#pragma endregion

	return !(app->threadHandles.hNotificationReceiver_NamedPipe == NULL);
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hMutex_QnA = CreateMutex(
		NULL,				//Security attributes
		FALSE,				//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_CommQnA	//Mutex name
	);

	syncHandles->hMutex_Toss = CreateMutex(
		NULL,				//Security attributes
		FALSE,				//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_CommToss	//Mutex name
	);

	return !(syncHandles->hMutex_QnA == NULL ||
		syncHandles->hMutex_Toss == NULL);
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseNamedPipeHandles(&app->namedPipeHandles);
	Setup_CloseSyncHandles(&app->syncHandles);
}

void Setup_CloseNamedPipeHandles(NamedPipeHandles* namedPipeHandles){
	CloseHandle(namedPipeHandles->hQnA);
	CloseHandle(namedPipeHandles->hRead);
	CloseHandle(namedPipeHandles->hWrite);
}

void Setup_CloseSyncHandles(SyncHandles* syncHandles){
	CloseHandle(syncHandles->hMutex_QnA);
	CloseHandle(syncHandles->hMutex_Toss);
}

void Service_GetMaxPass(Application* app){
	TParam_SendCommQnA* qnaParam = (TParam_SendCommQnA*) malloc(sizeof(TParam_SendCommQnA));
	if(qnaParam == NULL){
		_tprintf(TEXT("%sMalloc error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	CommsP2C sendComm;
	sendComm.commType = P2C_REQMAXPASS;

	qnaParam->app = app;
	qnaParam->commPC = sendComm;

	HANDLE hThread = CreateThread(
		NULL,				//Security Attributes
		0,					//Stack Size (0 = default)
		Thread_SendCommQnA,	//Function
		(LPVOID) qnaParam,	//Param
		0,					//Creation flags
		NULL);				//Thread Id

	WaitForSingleObject(hThread, INFINITE);
}

void Service_CloseApp(Application* app){
	TParam_SendCommToss* tossParam = (TParam_SendCommToss*) malloc(sizeof(TParam_SendCommToss));
	if(tossParam == NULL){
		_tprintf(TEXT("%sMalloc error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	CommsP2C sendComm;
	sendComm.commType = P2C_DISCONNECT;

	tossParam->app = app;
	tossParam->commPC = sendComm;

	HANDLE hThread = CreateThread(
		NULL,				//Security Attributes
		0,					//Stack Size (0 = default)
		Thread_SendCommToss,//Function
		(LPVOID) tossParam,	//Param
		0,					//Creation flags
		NULL);				//Thread Id

	WaitForSingleObject(hThread, INFINITE);

	Setup_CloseAllHandles(app);
	exit(1);
}

PassengerCommands Service_UseCommand(Application* app, TCHAR* command){
	if(_tcscmp(command, CMD_HELP) == 0){ //Continues on Main (listing commands)
		return PC_HELP;
	} else if(_tcscmp(command, CMD_LOGIN) == 0){ //Continues on Main (asking for new Passenger ID and XY coords)
		return PC_LOGIN;
	} else if(_tcscmp(command, CMD_LIST_PASSENGERS) == 0){ //Continues on Main (listing logged in passengers)
		return PC_LIST_PASSENGERS;
	} else if(_tcscmp(command, CMD_CLOSEAPP) == 0){
		Service_CloseApp(app);
		return PC_CLOSEAPP;
	}

	return PC_UNDEFINED;
}

bool Add_Passenger(Application* app, CommsP2C_Login* loginComm){
	/*No need for more validation...
	**Since it is assumed that this function is only called at Thread_SendCommQnA, which sends comm to Central which validates everything
	*/

	CPPassenger* anchorPass = &app->passengerList[Get_FreeIndexPassengerList(app)];
	if(anchorPass == NULL)
		return false;

	anchorPass->passengerInfo.empty = false;
	_tcscpy_s(anchorPass->passengerInfo.Id, _countof(anchorPass->passengerInfo.Id), loginComm->id);
	anchorPass->passengerInfo.object.xyPosition = loginComm->xyStartingPosition;
	anchorPass->xyDestination = loginComm->xyDestination;
	return true;
}
bool Delete_Passenger(Application* app, int index){
	if(index < 0 || index >= app->maxPass)
		return false;

	CPPassenger* anchorPassenger = Get_Passenger(app, index);
	if(anchorPassenger == NULL)
		return false;

	_tprintf(TEXT("%s[Passenger Logout] %s at (%.2lf, %.2lf) with intent of going to (%.2lf, %.2lf)"),
		Utils_NewSubLine(),
		anchorPassenger->passengerInfo.Id,
		anchorPassenger->passengerInfo.object.xyPosition.x,
		anchorPassenger->passengerInfo.object.xyPosition.y,
		anchorPassenger->xyDestination.x,
		anchorPassenger->xyDestination.y);

	anchorPassenger->passengerInfo.empty = true;

	return true;
}
int Get_QuantLoggedInPassengers(Application* app){
	int quantLoggedInPassengers = 0;

	for(int i = 0; i < app->maxPass; i++){
		if(!app->passengerList[i].passengerInfo.empty)
			quantLoggedInPassengers++;
	}

	return quantLoggedInPassengers;
}
bool isPassengerListFull(Application* app){
	return Get_QuantLoggedInPassengers(app) >= app->maxPass;
}
int Get_FreeIndexPassengerList(Application* app){
	if(isPassengerListFull(app))
		return -1;

	for(int i = 0; i < app->maxPass; i++){
		if(app->passengerList[i].passengerInfo.empty)
			return i;
	}

	return -1;
}
int Get_PassengerIndex(Application* app, TCHAR* Id){
	if(app->passengerList == NULL)
		return -1;

	for(int i = 0; i < app->maxPass; i++){
		if(_tcscmp(app->passengerList[i].passengerInfo.Id, Id) == 0 && !app->passengerList[i].passengerInfo.empty)
			return i;
	}

	return -1;
}
CPPassenger* Get_Passenger(Application* app, int index){
	if(app->passengerList == NULL)
		return NULL;

	if(!app->passengerList[index].passengerInfo.empty)
		return &app->passengerList[index];

	return NULL;
}

bool Command_LoginPassenger(Application* app, TCHAR* sId, TCHAR* sAtX, TCHAR* sAtY, TCHAR* sDestinyX, TCHAR* sDestinyY){
	if(Utils_StringIsEmpty(sId) ||
		Utils_StringIsEmpty(sAtX) ||
		Utils_StringIsEmpty(sAtY) ||
		Utils_StringIsEmpty(sDestinyX) ||
		Utils_StringIsEmpty(sDestinyY))
		return false;

	if(!(Utils_StringIsNumber(sAtX) &&
		Utils_StringIsNumber(sAtY) &&
		Utils_StringIsNumber(sDestinyX) &&
		Utils_StringIsNumber(sDestinyY)))
		return false;
	
	TParam_SendCommQnA* qnaParam = (TParam_SendCommQnA*) malloc(sizeof(TParam_SendCommQnA));
	if(qnaParam == NULL){
		_tprintf(TEXT("%sMalloc error: %d"), Utils_NewLine(), GetLastError());
		return false;
	}

	CommsP2C sendComm;
	sendComm.commType = P2C_LOGIN;
	_tcscpy_s(sendComm.loginComm.id, _countof(sendComm.loginComm.id), sId);
	sendComm.loginComm.xyStartingPosition.x = _ttoi(sAtX);
	sendComm.loginComm.xyStartingPosition.y = _ttoi(sAtY);
	sendComm.loginComm.xyDestination.x = _ttoi(sDestinyX);
	sendComm.loginComm.xyDestination.y = _ttoi(sDestinyY);
	
	qnaParam->app = app;
	qnaParam->commPC = sendComm;

	CreateThread(
		NULL,				//Security Attributes
		0,					//Stack Size (0 = default)
		Thread_SendCommQnA,	//Function
		(LPVOID) qnaParam,	//Param
		0,					//Creation flags
		NULL);				//Thread Id

	return true;
}