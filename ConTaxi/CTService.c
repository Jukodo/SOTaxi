#pragma once
#include "CTService.h"

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

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){
	TParam_QnARequest* request = (TParam_QnARequest*) malloc(sizeof(TParam_QnARequest));

	LoginRequest loginRequest;
	_tcscpy_s(loginRequest.licensePlate, _countof(loginRequest.licensePlate), sLicensePlate);
	loginRequest.coordX = (float) _tstof(sCoordinates_X);
	loginRequest.coordY = (float) _tstof(sCoordinates_Y);

	request->app = app;
	request->request.loginRequest = loginRequest;
	request->request.requestType = RT_LOGIN;

	app->threadHandles.hQnARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_SendQnARequests,				//Function
		(LPVOID) request,					//Param
		0,									//Creation Flag
		&app->threadHandles.dwIdQnARequests //Thread ID
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
	} else if(_tcscmp(command, CMD_DEFINE_CDN) == 0){ //Continues on Main (asking for value argument)
		return TC_DEFINE_CDN;
	} else if(_tcscmp(command, CMD_REQUEST_INTEREST) == 0){ //Continues on Main (asking for value argument)
		return TC_REQUEST_INTEREST;
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
	_tcscpy_s(ntIntRequest.licensePlate, _countof(ntIntRequest.idPassenger), app->loggedInTaxi.LicensePlate);

	param->app = app;
	param->request.ntIntRequest = ntIntRequest;
	param->request.requestType = RT_NT_INTEREST;

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

		if(app->loggedInTaxi.object.speedMultiplier == TOPMAX_SPEED){
			_tprintf(TEXT("%sCurrent speed (%.2lf) cannot go any higher!"), Utils_NewSubLine(), app->loggedInTaxi.object.speedMultiplier);
			return false;
		}

		app->loggedInTaxi.object.speedMultiplier += SPEED_CHANGEBY;
		_tprintf(TEXT("%sYour speed has been increased by %.2lf!%sYour current speed is %.2lf..."), Utils_NewSubLine(), SPEED_CHANGEBY, Utils_NewSubLine(), app->loggedInTaxi.object.speedMultiplier);
	} else{
		if(app->loggedInTaxi.object.speedMultiplier == TOPMIN_SPEED){
			_tprintf(TEXT("%sCurrent speed (%.2lf) cannot go any lower!"), Utils_NewSubLine(), app->loggedInTaxi.object.speedMultiplier);
			return false;
		}

		app->loggedInTaxi.object.speedMultiplier -= SPEED_CHANGEBY;
		_tprintf(TEXT("%sYour speed has been decreased by %.2lf!%sYour current speed is %.2lf..."), Utils_NewSubLine(), SPEED_CHANGEBY, Utils_NewSubLine(), app->loggedInTaxi.object.speedMultiplier);
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