#pragma once
#include "CTCommunication.h"

DWORD WINAPI Thread_SendLARequests(LPVOID request){
	TParam_LARequest* req = (TParam_LARequest*) request;
	LARequest* shm = req->app->shmHandles.lpSHM_LARequest;

	WaitForSingleObject(req->app->syncHandles.hMutex_LARequest, INFINITE);
	WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Write, INFINITE);
	switch(req->request.requestType){
		case RT_LOGIN:
			CopyMemory(shm, &req->request, sizeof(LARequest));
			SetEvent(req->app->syncHandles.hEvent_LARequest_Read);

			WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Write, INFINITE);
			
			switch(shm->loginResponse){
				case LR_SUCCESS:
					req->app->loggedInTaxi.empty = false;
					_tcscpy_s(req->app->loggedInTaxi.LicensePlate, _countof(req->app->loggedInTaxi.LicensePlate), req->request.loginRequest.licensePlate);
					req->app->loggedInTaxi.object.coordX = req->request.loginRequest.coordX;
					req->app->loggedInTaxi.object.coordY = req->request.loginRequest.coordY;
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
			CopyMemory(shm, &req->request, sizeof(LARequest));
			SetEvent(req->app->syncHandles.hEvent_LARequest_Read);

			WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Write, INFINITE);

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
	}

	ReleaseMutex(req->app->syncHandles.hMutex_LARequest);
	SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
	free(req);
	return 1;
}