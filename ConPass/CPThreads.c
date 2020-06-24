#pragma once
#include "CPThreads.h"

DWORD WINAPI Thread_NotificationReceiver_NamedPipe(LPVOID _param){
	TParam_NotificationReceiver_NamedPipe* param = (TParam_NotificationReceiver_NamedPipe*) _param;

	CommsC2P receivedComm;
	while(param->app->keepRunning){
		ReadFile(param->app->namedPipeHandles.hRead,	//Named pipe handle
			&receivedComm,								//Read into
			sizeof(CommsC2P),							//Size being read
			NULL,										//Quantity of bytes read
			NULL);										//Overlapped IO

		switch(receivedComm.commType){
			case C2P_ASSIGNED:
				_tprintf(TEXT("%s[Taxi Assignment] Passenger %s has been assigned to Taxi %s and is estimated to take %d seconds till arrival..."),
					Utils_NewSubLine(),
					receivedComm.assignComm.passId,
					receivedComm.assignComm.licensePlate,
					receivedComm.assignComm.estimatedWaitTime);
				break;
			case C2P_ASSIGNED_FAILED:
				_tprintf(TEXT("%s[Taxi Assignment] Passenger %s transport request had no taxis interested, logging out..."),
					Utils_NewSubLine(),
					receivedComm.assignComm.passId);
				break;
			case C2P_PASS_REMOVAL:
				Delete_Passenger(param->app, Get_PassengerIndex(param->app, receivedComm.removeComm.passId));
				break;
			case C2P_PASS_ARRIVED:
				_tprintf(TEXT("%s[Taxi Arrival] Passenger %s has arrived to its destination!"),
					Utils_NewSubLine(),
					receivedComm.arrivedComm.passId);
				break;
			case C2P_SHUTDOWN:
				_tprintf(TEXT("%sReceived a Shutdown Comm"), Utils_NewSubLine());
				break;
		}
	}

	free(param);
	return 301;
}

DWORD WINAPI Thread_SendCommQnA(LPVOID _param){
	TParam_SendCommQnA* param = (TParam_SendCommQnA*) _param;
	
	WaitForSingleObject(param->app->syncHandles.hMutex_QnA, INFINITE);

	WriteFile(
		param->app->namedPipeHandles.hQnA,	//Named pipe handle
		&param->commPC,						//Write from 
		sizeof(CommsP2C),					//Size being written
		NULL,								//Quantity Bytes written
		NULL);								//Overlapped IO

	CommsC2P responseComm;
	ReadFile(param->app->namedPipeHandles.hQnA,	//Named pipe handle
		&responseComm,							//Read into
		sizeof(CommsC2P),						//Size being read
		NULL,									//Quantity of bytes read
		NULL);									//Overlapped IO

	switch(responseComm.commType){
		case C2P_RESP_LOGIN:
			switch(responseComm.loginRespComm){
			case PLR_SUCCESS:
				_tprintf(TEXT("%sSuccess! [%s] login has been registered successfully!"), Utils_NewSubLine(), param->commPC.loginComm.id);
				if(!Add_Passenger(param->app, &param->commPC.loginComm))
					_tprintf(TEXT("%sSomething unexpected happened! Maybe local passList is full and tried to add more..."), Utils_NewSubLine());
				else
					_tprintf(TEXT("% sTransport of the passenger is being taken care of!"), Utils_NewSubLine());
				break;
			case PLR_INVALID_UNDEFINED:
				_tprintf(TEXT("%sError... [%s] login has been rejected!%sPlease try again!"), Utils_NewSubLine(), param->commPC.loginComm.id, Utils_NewSubLine());
				break;
			case PLR_INVALID_TRANSPBUFFER_FULL:
				_tprintf(TEXT("%sError... [%s] login has been rejected!%sTransport buffer is full! Try again later..."), Utils_NewSubLine(), param->commPC.loginComm.id, Utils_NewSubLine());
				break;
			case PLR_INVALID_FULL:
				_tprintf(TEXT("%sError... [%s] login has been rejected!%sThe application doesn't accept more passengers!"), Utils_NewSubLine(), param->commPC.loginComm.id, Utils_NewSubLine());
				break;
			case PLR_INVALID_POSITION:
				_tprintf(TEXT("%sError... [%s] login has been rejected!%sThe position chosen is invalid!"), Utils_NewSubLine(), param->commPC.loginComm.id, Utils_NewSubLine());
				break;
			case PLR_INVALID_DESTINY:
				_tprintf(TEXT("%sError... [%s] login has been rejected!%sThe destination position chosen is invalid!"), Utils_NewSubLine(), param->commPC.loginComm.id, Utils_NewSubLine());
				break;
			case PLR_INVALID_EXISTS:
				_tprintf(TEXT("%sError... [%s] login has been rejected!%sThe id chosen is already exists!"), Utils_NewSubLine(), param->commPC.loginComm.id, Utils_NewSubLine());
				break;
			default:
				_tprintf(TEXT("%sOh no! [%s] login has been rejected!%sThis error was unexpected! Error: %d"), Utils_NewSubLine(), param->commPC.loginComm.id, Utils_NewSubLine(), GetLastError());
				break;
			}
			break;
		case C2P_RESP_MAXPASS:
			param->app->maxPass = responseComm.maxPassRespComm.maxPass;
			break;
		default:
			_tprintf(TEXT("%sQnA Response Wtf is this"), Utils_NewSubLine());
	}

	ReleaseMutex(param->app->syncHandles.hMutex_QnA);
	
	free(param);
	return 302;
}

DWORD WINAPI Thread_SendCommToss(LPVOID _param){
	TParam_SendCommToss* param = (TParam_SendCommToss*) _param;

	WaitForSingleObject(param->app->syncHandles.hMutex_Toss, INFINITE);

	WriteFile(
		param->app->namedPipeHandles.hWrite,//Named pipe handle
		&param->commPC,						//Write from 
		sizeof(CommsP2C),					//Size being written
		NULL,								//Quantity Bytes written
		NULL);								//Overlapped IO

	ReleaseMutex(param->app->syncHandles.hMutex_Toss);

	free(param);
	return 303;
}