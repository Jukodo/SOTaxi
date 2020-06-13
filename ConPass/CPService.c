#pragma once
#include "CPService.h"

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->keepRunning = true;


	bool ret = true;
	ret = ret && Setup_OpenThreadHandles(&app->threadHandles);

	return ret;
}

bool Setup_OpenThreadHandles(ThreadHandles* threadHandles){

	return true;
}

void Setup_CloseAllHandles(Application* app){

}

PassengerCommands Service_UseCommand(Application* app, TCHAR* command){
	if(_tcscmp(command, CMD_HELP) == 0){ //Continues on Main (listing commands)
		return PC_HELP;
	} else if(_tcscmp(command, CMD_LOGIN) == 0){ //Continues on Main (asking for new Passenger ID and XY coords)
		return PC_LOGIN;
	} else if(_tcscmp(command, CMD_LIST_PASSENGERS) == 0){
		Command_ListPassengers(app);
		return PC_LIST_PASSENGERS;
	} else if(_tcscmp(command, CMD_CLOSEAPP) == 0){
		Service_CloseApp(app);
		return PC_CLOSEAPP;
	}

	return PC_UNDEFINED;
}

void Service_CloseApp(Application* app){

}

bool Command_LoginPassenger(Application* app, TCHAR* sId, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){

	return false;
}

void Command_ListPassengers(Application* app){

}