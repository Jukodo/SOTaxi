#pragma once
#include "CTService.h"

bool isLoggedIn(Application* app){
	return !app->loggedInTaxi.taxiInfo.empty;
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

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){
	TParam_QnARequest* param = (TParam_QnARequest*) malloc(sizeof(TParam_QnARequest));

	LoginRequest loginRequest;
	_tcscpy_s(loginRequest.licensePlate, _countof(loginRequest.licensePlate), sLicensePlate);
	loginRequest.coordX = (float) _tstof(sCoordinates_X);
	loginRequest.coordY = (float) _tstof(sCoordinates_Y);

	param->app = app;
	param->request.loginRequest = loginRequest;
	param->request.requestType = QnART_LOGIN;

	app->threadHandles.hQnARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendQnARequests,				//Function
		(LPVOID) param,						//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdQnARequests //Thread ID
	);
}

bool Service_PosLoginSetup(Application* app){
	if(!Service_ConnectToCentralNamedPipe(app)){
		_tprintf(TEXT("%sFailed while trying to connect to central's named pipe!"), Utils_NewSubLine());
		return false;
	}

	app->NTBuffer_Tail = ((NewTransportBuffer*) app->shmHandles.lpSHM_NTBuffer)->head; //Makes sure taxi starts its NewTransport buffer queue from current start (head)
	ResumeThread(app->threadHandles.hNotificationReceiver_NewTransport); //Allows NewTransport notifications to start popping up
	ResumeThread(app->threadHandles.hNotificationReceiver_NamedPipe); //Allows NamedPipe notifications to start popping up
	app->loggedInTaxi.taxiInfo.object.speedX = 1;
	app->loggedInTaxi.taxiInfo.object.speedY = 0;
	
	TParam_StepRoutine* srParam = (TParam_StepRoutine*) malloc(sizeof(TParam_StepRoutine));
	srParam->app = app;
	app->threadHandles.hStepRoutine = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_StepRoutine,					//Function
		(LPVOID) srParam,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdStepRoutine //Thread ID
	);

	return true;
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
	} else if(_tcscmp(command, CMD_DEFINE_CDN) == 0){ //Continues on Main (asking for value argument)
		return TC_DEFINE_CDN;
	} else if(_tcscmp(command, CMD_REQUEST_INTEREST) == 0){ //Continues on Main (asking for value argument)
		return TC_REQUEST_INTEREST;
	} else if(_tcscmp(command, CMD_SHOW_MAP) == 0){
		Temp_ShowMap(app);
		return TC_SHOW_MAP;
	} else if(_tcscmp(command, CMD_CLOSEAPP) == 0){
		Service_CloseApp(app);
		return TC_CLOSEAPP;
	}

	return TC_UNDEFINED;
}

void Service_RegisterInterest(Application* app, TCHAR* idPassenger){
	TParam_QnARequest* param = (TParam_QnARequest*) malloc(sizeof(TParam_QnARequest));

	NTInterestRequest ntIntRequest;
	_tcscpy_s(ntIntRequest.idPassenger, _countof(ntIntRequest.idPassenger), idPassenger);
	_tcscpy_s(ntIntRequest.licensePlate, _countof(ntIntRequest.idPassenger), app->loggedInTaxi.taxiInfo.LicensePlate);

	param->app = app;
	param->request.ntIntRequest = ntIntRequest;
	param->request.requestType = QnART_NT_INTEREST;

	app->threadHandles.hQnARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendQnARequests,				//Function
		(LPVOID) param,						//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdQnARequests //Thread ID
	);
}

void Service_CloseApp(Application* app){
	/*ToDo (TAG_TODO)
	**Notify central about this shutdown, in order to logout taxi from central
	*/
}

void Service_NewPosition(Application* app, double newX, double newY){
	TParam_TossRequest* param = (TParam_TossRequest*) malloc(sizeof(TParam_TossRequest));

	TossPosition tossPosition;
	_tcscpy_s(tossPosition.licensePlate, _countof(tossPosition.licensePlate), app->loggedInTaxi.taxiInfo.LicensePlate);
	tossPosition.newX = newX;
	tossPosition.newY = newY;

	param->app = app;
	param->tossRequest.tossPosition = tossPosition;
	param->tossRequest.tossType = TRT_TAXI_POSITION;

	app->threadHandles.hTossRequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_TossRequest,				//Function
		(LPVOID) param,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdTossRequests //Thread ID
	);
}

void Service_NewState(Application* app, TaxiState newState){
	TParam_TossRequest* param = (TParam_TossRequest*) malloc(sizeof(TParam_TossRequest));

	TossState tossState;
	_tcscpy_s(tossState.licensePlate, _countof(tossState.licensePlate), app->loggedInTaxi.taxiInfo.LicensePlate);
	tossState.newState = newState;

	param->app = app;
	param->tossRequest.tossState = tossState;
	param->tossRequest.tossType = TRT_TAXI_STATE;

	app->threadHandles.hTossRequests = CreateThread(
		NULL,									//Security Attributes
		0,										//Stack Size (0 = default)
		Thread_TossRequest,						//Function
		(LPVOID) param,							//Param
		0,										//Creation Flag
		&app->threadHandles.dwIdTossRequests	//Thread ID
	);
}

bool Command_DefineCDN(Application* app, TCHAR* value){
	if(!Utils_StringIsNumber(value))
		return false;

	int cdnValue = _ttoi(value);
	if(cdnValue <= 0 || cdnValue > TOPMAX_CDN){
		_tprintf(TEXT("%sCDN value has to be between 1 and %d! Hence, it remained the same (%d)..."), Utils_NewSubLine(), TOPMAX_CDN, app->settings.CDN);
		return true;
	}

	app->settings.CDN = cdnValue;
	_tprintf(TEXT("%sCDN value has been changed to %d!"), Utils_NewSubLine(), app->settings.CDN);
	return true;
}

bool Command_Speed(Application* app, bool speedUp){
	if(speedUp){

		if(app->loggedInTaxi.taxiInfo.object.speedMultiplier == TOPMAX_SPEED){
			_tprintf(TEXT("%sCurrent speed (%.2lf) cannot go any higher!"), Utils_NewSubLine(), app->loggedInTaxi.taxiInfo.object.speedMultiplier);
			return false;
		}

		app->loggedInTaxi.taxiInfo.object.speedMultiplier += SPEED_CHANGEBY;
		_tprintf(TEXT("%sYour speed has been increased by %.2lf!%sYour current speed is %.2lf..."), Utils_NewSubLine(), SPEED_CHANGEBY, Utils_NewSubLine(), app->loggedInTaxi.taxiInfo.object.speedMultiplier);
	} else{
		if(app->loggedInTaxi.taxiInfo.object.speedMultiplier == TOPMIN_SPEED){
			_tprintf(TEXT("%sCurrent speed (%.2lf) cannot go any lower!"), Utils_NewSubLine(), app->loggedInTaxi.taxiInfo.object.speedMultiplier);
			return false;
		}

		app->loggedInTaxi.taxiInfo.object.speedMultiplier -= SPEED_CHANGEBY;
		_tprintf(TEXT("%sYour speed has been decreased by %.2lf!%sYour current speed is %.2lf..."), Utils_NewSubLine(), SPEED_CHANGEBY, Utils_NewSubLine(), app->loggedInTaxi.taxiInfo.object.speedMultiplier);
	}

	return true;
}

void Command_AutoResp(Application* app, bool autoResp){
	app->settings.automaticInterest = autoResp;
	
	if(app->settings.automaticInterest)
		_tprintf(TEXT("%sAutomatic interest requests is now ON!"), Utils_NewSubLine());
	else
		_tprintf(TEXT("%sAutomatic interest requests is now OFF!"), Utils_NewSubLine());
}

DWORD WINAPI Thread_StepRoutine(LPVOID _param){
	TParam_StepRoutine* param = (TParam_StepRoutine*) _param;

	LARGE_INTEGER liTime;
	liTime.QuadPart = -10000000LL * 1 /*Triggers after 1 second*/;
	SetWaitableTimer(param->app->taxiMovementRoutine, &liTime, 1000, NULL, NULL, FALSE);
	while(param->app->keepRunning){
		WaitForSingleObject(param->app->taxiMovementRoutine, INFINITE);

		CTTaxi* loggedInTaxi = &param->app->loggedInTaxi;
		if(param->app->loggedInTaxi.taxiInfo.state == TS_EMPTY){
			if(Movement_NextRandomStep(param->app, &loggedInTaxi->taxiInfo.object)){
				Service_NewPosition(param->app, loggedInTaxi->taxiInfo.object.coordX, loggedInTaxi->taxiInfo.object.coordY);
			}
		}
	}

	free(param);
	return 1;
}

bool Movement_NextRandomStep(Application* app, XYObject* object){
	if((object->speedX * object->speedY) != 0){ //Doesn't allow diagonal movement
		_tprintf(TEXT("%sTrying to move diagonally SpeedX:%.2lf SpeedY:%.2lf"), Utils_NewSubLine(), object->speedX, object->speedY);
		return false;
	}

	double nextX = object->coordX + (object->speedX * object->speedMultiplier);
	double nextY = object->coordY + (object->speedY * object->speedMultiplier);
	if(nextX < 0 || nextX >= app->map.width){
		_tprintf(TEXT("%sX out of bounds X:%.2lf MaxX:%d"), Utils_NewSubLine(), nextX, app->map.width-1);
		return false;
	}
	if(nextY < 0 || nextY >= app->map.height){
		_tprintf(TEXT("%sY out of bounds Y:%.2lf MaxY:%d"), Utils_NewSubLine(), nextY, app->map.height-1);
		return false;
	}

	object->coordX = nextX;
	object->coordY = nextY;
	return true;
}

void Temp_ShowMap(Application* app){
	int iLine = 0;
	int iColumn = 0;
	for(int i = 0; i < (app->map.height * app->map.width); i++){
		iColumn = i % app->map.width;
		iLine = i / app->map.height;
		if(iColumn == 0)
			_tprintf(TEXT("\n"));

		_tprintf(TEXT("%c"), app->map.cellArray[i]);
	}
}