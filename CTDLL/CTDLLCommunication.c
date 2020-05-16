#pragma once
#include "CTDLLCommunication.h"

DWORD WINAPI Thread_SendLARequests(LPVOID _param){
	TParam_LARequest* param = (TParam_LARequest*) _param;
	LARequest* shm = param->app->shmHandles.lpSHM_LARequest;

	WaitForSingleObject(param->app->syncHandles.hMutex_LARequest, INFINITE);
	WaitForSingleObject(param->app->syncHandles.hEvent_LARequest_Write, INFINITE);

	CopyMemory(shm, &param->request, sizeof(LARequest));
	SetEvent(param->app->syncHandles.hEvent_LARequest_Read);

	WaitForSingleObject(param->app->syncHandles.hEvent_LARequest_Write, INFINITE);

	switch(param->request.requestType){
	case RT_LOGIN:

		switch(shm->loginResponse){
		case LR_SUCCESS:
			param->app->loggedInTaxi.empty = false;
			_tcscpy_s(param->app->loggedInTaxi.LicensePlate, _countof(param->app->loggedInTaxi.LicensePlate), param->request.loginRequest.licensePlate);
			param->app->loggedInTaxi.object.coordX = param->request.loginRequest.coordX;
			param->app->loggedInTaxi.object.coordY = param->request.loginRequest.coordY;
			break;
		case LR_INVALID_UNDEFINED:
			_tprintf(TEXT("%sError! Please try again..."), Utils_NewSubLine());
			break;
		case LR_INVALID_FULL:
			_tprintf(TEXT("%sError! The application doesn't accept more taxis"), Utils_NewSubLine());
			break;
		case LR_INVALID_POSITION:
			_tprintf(TEXT("%sError! The position chosen is invalid"), Utils_NewSubLine());
			break;
		}

		break;

	case RT_ASSIGN:

		switch(shm->assignResponse){
		case AR_SUCCESS:
			_tprintf(TEXT("%sSuccess! But that ain't done yet..."), Utils_NewSubLine());
			break;
		case AR_INVALID_UNDEFINED:
			_tprintf(TEXT("%sError! That ain't done yet..."), Utils_NewSubLine());
			break;
		default:
			_tprintf(TEXT("%sOh no! Not expecting this error!"), Utils_NewSubLine());
			break;
		}

		break;

	case RT_VAR:
		if(shm->varResponse.maxTaxis <= 0 || shm->varResponse.maxTaxis > TOPMAX_TAXI)
			shm->varResponse.maxTaxis = -1;

		if(shm->varResponse.maxPassengers <= 0 || shm->varResponse.maxPassengers > TOPMAX_PASSENGERS)
			shm->varResponse.maxPassengers = -1;

		param->app->maxTaxis = shm->varResponse.maxTaxis;
		param->app->maxPassengers = shm->varResponse.maxPassengers;

		break;
	}

	ReleaseMutex(param->app->syncHandles.hMutex_LARequest);
	SetEvent(param->app->syncHandles.hEvent_LARequest_Write);
	free(param);
	return 1;
}

DWORD WINAPI Thread_NotificationReceiver_NP(LPVOID _param){
	Application* app = (Application*) _param;

	while(true){
		WaitForSingleObject(app->syncHandles.hEvent_Notify_T_NP, INFINITE);
		app->quant++;
		_tprintf(TEXT("%sI got notified %d times"), Utils_NewLine(), app->quant);
	}
}