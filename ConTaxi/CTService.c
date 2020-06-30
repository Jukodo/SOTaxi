#pragma once
#include "CTService.h"
#include "CTThreads.h"

bool isLoggedIn(Application* app){
	return !app->loggedInTaxi.taxiInfo.empty;
}

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){
	TParam_QnARequest* param = (TParam_QnARequest*) malloc(sizeof(TParam_QnARequest));

	TaxiLoginRequest taxiLoginRequest;
	_tcscpy_s(taxiLoginRequest.licensePlate, _countof(taxiLoginRequest.licensePlate), sLicensePlate);
	taxiLoginRequest.xyStartingPosition.x = _tstof(sCoordinates_X);
	taxiLoginRequest.xyStartingPosition.y = _tstof(sCoordinates_Y);

	param->app = app;
	param->request.taxiLoginRequest = taxiLoginRequest;
	param->request.requestType = QnART_LOGIN;

	bool loopAgain = false;
	do{
		app->threadHandles.hQnARequests = CreateThread(
			NULL,								//Security Attributes
			0,									//Stack Size (0 = default)
			Thread_SendQnARequests,				//Function
			(LPVOID) param,						//Param
			0,									//Creation Flag
			&app->threadHandles.dwIdQnARequests //Thread ID
		);

		WaitForSingleObject(app->threadHandles.hQnARequests, INFINITE);

		if(param->request.taxiLoginResponse.taxiLoginResponseType != TLR_INVALID_FULL)
			break;

		loopAgain = Service_LoginQueue(app);
	} while(loopAgain);

	free(param);
}
bool Service_PosLoginSetup(Application* app){
	if(!Service_ConnectToCentralNamedPipe(app)){
		_tprintf(TEXT("%sFailed while trying to connect to central's named pipe!"), Utils_NewSubLine());
		return false;
	}

	app->NTBuffer_Tail = ((TransportBuffer*) app->shmHandles.lpSHM_NTBuffer)->head; //Makes sure taxi starts its NewTransport buffer queue from current start (head)
	ResumeThread(app->threadHandles.hNotificationReceiver_NewTransport); //Allows NewTransport notifications to start popping up
	ResumeThread(app->threadHandles.hNotificationReceiver_NamedPipe); //Allows NamedPipe notifications to start popping up
	app->loggedInTaxi.taxiInfo.object.speed.x = 1;
	app->loggedInTaxi.taxiInfo.object.speed.y = 0;
	app->stepRoutine.hStepRoutine = NULL;

	TParam_DestinationChanger* dgParam = (TParam_DestinationChanger*) malloc(sizeof(TParam_DestinationChanger));
	dgParam->app = app;
	app->threadHandles.hDestinationChanger = CreateThread(
		NULL,										//Security Attributes
		0,											//Stack Size (0 = default)
		Thread_DestinationChanger,					//Function
		(LPVOID) dgParam,							//Param
		0,											//Creation Flag
		&app->threadHandles.dwIdDestinationChanger	//Thread ID
	);

	if(app->threadHandles.hDestinationChanger == NULL)
		return false;

	TParam_StepRoutine* param = malloc(sizeof(TParam_StepRoutine));
	param->app = app;

	app->stepRoutine.hStepRoutine = CreateThread(
		NULL,									//Security Attributes
		0,										//Stack Size (0 = default)
		Thread_StepRoutine,						//Function
		(LPVOID) param,							//Param
		0,										//Creation Flag
		&app->stepRoutine.dwIdStepRoutine);		//Thread ID

	if(app->stepRoutine.hStepRoutine == NULL)
		return false;

	return true;
}
bool Service_LoginQueue(Application* app){
	_tprintf(TEXT("%sYou are now in queue to login into the central! Timeout after %d seconds..."), Utils_NewSubLine(), TIMEOUT_TaxiLoginQueue_Seconds);
	
	switch(WaitForSingleObject(app->syncHandles.hEvent_NewTaxiSpot, TIMEOUT_TaxiLoginQueue)){
		case WAIT_OBJECT_0:
			_tprintf(TEXT("%sA new spot to login was found!"), Utils_NewSubLine());
			return true;
		case WAIT_TIMEOUT:
			_tprintf(TEXT("%sLogin queue was timed out..."), Utils_NewSubLine());
			break;
		default:
			_tprintf(TEXT("%sSomething unexpected went wrong..."), Utils_NewSubLine());
			break;
	}

	return false;
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
	app->keepRunning = false;

	TParam_TossRequest* param = (TParam_TossRequest*) malloc(sizeof(TParam_TossRequest));

	TossRequest tossRequest;
	tossRequest.tossType = TRT_TAXI_LOGOUT;
	_tcscpy_s(tossRequest.tossLogout.licensePlate, _countof(tossRequest.tossLogout.licensePlate), app->loggedInTaxi.taxiInfo.LicensePlate);

	param->app = app;
	param->tossRequest = tossRequest;

	app->threadHandles.hTossRequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_TossRequest,				//Function
		(LPVOID) param,						//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdTossRequests //Thread ID
	);
	WaitForSingleObject(app->threadHandles.hTossRequests, INFINITE);
}
void Service_NewPosition(Application* app, XY xyNewPosition){
	if(app->loggedInTaxi.taxiInfo.empty ||
		(xyNewPosition.x < 0 || xyNewPosition.x >= app->map.width) &&
		(xyNewPosition.y < 0 || xyNewPosition.y >= app->map.height))
		return;

	app->loggedInTaxi.taxiInfo.object.xyPosition = xyNewPosition;

	TossRequest tossRequest;
	TossPosition tossPosition;
	_tcscpy_s(tossPosition.licensePlate, _countof(tossPosition.licensePlate), app->loggedInTaxi.taxiInfo.LicensePlate);
	tossPosition.xyNewPosition = xyNewPosition;

	tossRequest.tossPosition = tossPosition;
	tossRequest.tossType = TRT_TAXI_POSITION;

	Communication_SendTossRequest(app, tossRequest);
}
void Service_NewState(Application* app, TaxiState newState){
	app->loggedInTaxi.taxiInfo.state = newState;

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
		&app->threadHandles.dwIdTossRequests);	//Thread ID
}

bool Service_NewDestination(Application* app, XY xyDestination){
	if(app->loggedInTaxi.taxiInfo.empty)
		return false;

	WaitForSingleObject(app->syncHandles.hMutex_StepRoutine, INFINITE);

	if(app->stepRoutine.path != NULL){
		free(app->stepRoutine.path->path);
		free(app->stepRoutine.path);
	}

	app->stepRoutine.path = Utils_GetPath(&app->map, app->loggedInTaxi.taxiInfo.object.xyPosition, xyDestination);
	app->stepRoutine.currentStep = 0;

	/*Check if thread isn't alive
	**If not, create a thread for the routine
	*/
	if(app->stepRoutine.hStepRoutine == NULL || WaitForSingleObject(app->stepRoutine.hStepRoutine, 0) == WAIT_OBJECT_0){
		TParam_StepRoutine* param = malloc(sizeof(TParam_StepRoutine));
		param->app = app;

		app->stepRoutine.hStepRoutine = CreateThread(
			NULL,									//Security Attributes
			0,										//Stack Size (0 = default)
			Thread_StepRoutine,						//Function
			(LPVOID) param,							//Param
			0,										//Creation Flag
			&app->stepRoutine.dwIdStepRoutine);		//Thread ID

		if(app->stepRoutine.hStepRoutine == NULL)
			return false;
	}

	ReleaseMutex(app->syncHandles.hMutex_StepRoutine);
	return true;
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

	//Send new Speed to CenTaxi
	TParam_TossRequest* param = (TParam_TossRequest*) malloc(sizeof(TParam_TossRequest));
	TossRequest tossRequest;
	tossRequest.tossType = TRT_TAXI_SPEED;
	_tcscpy_s(tossRequest.tossSpeed.licensePlate, _countof(tossRequest.tossLogout.licensePlate), app->loggedInTaxi.taxiInfo.LicensePlate);
	tossRequest.tossSpeed.newSpeed = app->loggedInTaxi.taxiInfo.object.speedMultiplier;

	param->app = app;
	param->tossRequest = tossRequest;

	app->threadHandles.hTossRequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_TossRequest,					//Function
		(LPVOID) param,						//Param
		0,									//Creation Flag
		NULL								//Thread ID
	);

	return true;
}
void Command_AutoResp(Application* app, bool autoResp){
	app->settings.automaticInterest = autoResp;
	
	if(app->settings.automaticInterest)
		_tprintf(TEXT("%sAutomatic interest requests is now ON!"), Utils_NewSubLine());
	else
		_tprintf(TEXT("%sAutomatic interest requests is now OFF!"), Utils_NewSubLine());
}

XY Movement_NextRandomStep(Application* app, XYObject* object){
	XY returnXY;
	returnXY.x = -1;
	returnXY.y = -1;

	if((object->speed.x * object->speed.y) != 0){ //Doesn't allow diagonal movement
		_tprintf(TEXT("%sTrying to move diagonally or stopped SpeedX:%.2lf SpeedY:%.2lf"), 
			Utils_NewSubLine(),
			object->speed.x, 
			object->speed.y);
		object->speed.x = 1;
		object->speed.y = 0;
		return returnXY;
	}

	XY tempXY;
	tempXY.x = object->xyPosition.x;
	tempXY.y = object->xyPosition.y;
	XY* neighbors4 = Utils_GetNeighbors4(&app->map, tempXY);
	XY* allowedList = NULL;

	int allowedNeighbors;

	for(int i = 0; i < 2; i++){
		allowedNeighbors = 0;
		for(int n = 0; n < MAX_NEIGHBORS; n++){
			if(neighbors4[n].x == -1 || neighbors4[n].y == -1) //Invalid neighbor (out of bounds or structure)
				continue;

			//Check if neighbor is the cell before the actual one, avoiding looping back and forth
			if(ceil(neighbors4[n].x) == ceil(object->xyPosition.x - (object->speed.x * object->speedMultiplier)) &&
				ceil(neighbors4[n].y) == ceil(object->xyPosition.y - (object->speed.y * object->speedMultiplier)))
				continue;


			if(allowedList != NULL)
				allowedList[allowedNeighbors] = neighbors4[n];

			allowedNeighbors++;
		}

		if(i == 0 && allowedNeighbors > 0){
			allowedList = calloc(allowedNeighbors, sizeof(XY));
			if(allowedList == NULL)
				return returnXY;
		}
	}

	if(allowedNeighbors > 0){
		returnXY = allowedList[rand()%allowedNeighbors];
	}

	if(allowedList != NULL){
		free(allowedList);
		allowedList = NULL;
	}

	if(neighbors4 != NULL){
		free(neighbors4);
		neighbors4 = NULL;
	}

	if(returnXY.x < 0 || returnXY.x >= app->map.width){
		_tprintf(TEXT("%sX out of bounds X:%.2lf MaxX:%d"), Utils_NewSubLine(), returnXY.x, app->map.width-1);
		return returnXY;
	}
	if(returnXY.y < 0 || returnXY.y >= app->map.height){
		_tprintf(TEXT("%sY out of bounds Y:%.2lf MaxY:%d"), Utils_NewSubLine(), returnXY.y, app->map.height-1);
		return returnXY;
	}

	if(object->xyPosition.x - returnXY.x < 0)//New X is farther to the right
		object->speed.x = 1;
	else if(object->xyPosition.x - returnXY.x > 0)//New X is farther to the left
		object->speed.x = -1;
	else
		object->speed.x = 0;
	if(object->xyPosition.y - returnXY.y < 0)//New Y is farther to the bottom
		object->speed.y = 1;
	else if(object->xyPosition.y - returnXY.y > 0)//New Y is farther to the top
		object->speed.y = -1;
	else
		object->speed.y = 0;

	return returnXY;
}

/* ToDo (TAG_REMOVE)
** Remove the following after
** Only used to develop and test few features
*/
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