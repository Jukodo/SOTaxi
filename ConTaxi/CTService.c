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
		Command_CloseApp(app);
		return TC_CLOSEAPP;
	}

	return TC_UNDEFINED;
}

void Service_RequestPass(Application* app, TCHAR* idPassenger){
	TParam_QnARequest* param = (TParam_QnARequest*) malloc(sizeof(TParam_QnARequest));

	NTInterestRequest ntIntRequest;
	_tcscpy_s(ntIntRequest.idPassenger, _countof(ntIntRequest.idPassenger), idPassenger);

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

bool Service_DefineCDN(Application* app, TCHAR* value){
	if(!Utils_StringIsNumber(value))
		return false;

	int cdnValue = _ttoi(value);
	//ToDo
	return true;
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