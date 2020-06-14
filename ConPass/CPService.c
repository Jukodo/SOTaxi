#pragma once
#include "CPService.h"

bool Setup_Application(Application* app){
	ZeroMemory(app, sizeof(Application));
	app->keepRunning = true;


	bool ret = true;
	ret = ret && Setup_OpenNamedPipeHandles(&app->namedPipeHandles);
	ret = ret && Setup_OpenThreadHandles(&app->threadHandles);

	return ret;
}

bool Setup_OpenNamedPipeHandles(NamedPipeHandles* namedPipeHandles){
	#pragma region ReadFromCentral
	HANDLE hPipe_Read = CreateFile(
		NAME_NAMEDPIPE_CommsPassCentral_C2P,//Named Pipe name
		GENERIC_READ,						//Read Access
		0,									//Doesn't share
		NULL,								//Security attributes
		OPEN_EXISTING,						//Open existing pipe 
		0,									//Atributes
		NULL);								//Template file 

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
		NAME_NAMEDPIPE_CommsPassCentral_P2C,//Named Pipe name
		GENERIC_WRITE,						//Read Access
		0,									//Doesn't share
		NULL,								//Security attributes
		OPEN_EXISTING,						//Open existing pipe 
		0,									//Atributes
		NULL);								//Template file 

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

	return true;
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
	CommsP2C commP2C;
	commP2C.commType = P2C_DISCONNECT;

	WriteFile(
		app->namedPipeHandles.hWrite,	//Named pipe handle
		&commP2C,							//Write from 
		sizeof(CommsP2C),						//Size being written
		NULL,									//Quantity Bytes written
		NULL);									//Overlapped IO
}

bool Command_LoginPassenger(Application* app, TCHAR* sId, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y){
	CommsP2C commP2C;
	commP2C.commType = P2C_LOGIN;

	WriteFile(
		app->namedPipeHandles.hWrite,	//Named pipe handle
		&commP2C,							//Write from 
		sizeof(CommsP2C),						//Size being written
		NULL,									//Quantity Bytes written
		NULL);									//Overlapped IO

	return true;
}

void Command_ListPassengers(Application* app){
	CommsP2C commP2C;
	commP2C.commType = P2C_REQMAXPASS;

	WriteFile(
		app->namedPipeHandles.hWrite,	//Named pipe handle
		&commP2C,							//Write from 
		sizeof(CommsP2C),						//Size being written
		NULL,									//Quantity Bytes written
		NULL);									//Overlapped IO
}