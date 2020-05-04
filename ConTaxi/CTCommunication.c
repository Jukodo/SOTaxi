#pragma once
#include "CTCommunication.h"

DWORD WINAPI Thread_SendLARequests(LPVOID request){
	TParam_LARequest* req = (TParam_LARequest*) request;
	LARequest* shm = req->app->shmHandles.lpSHM_LARequest;

	WaitForSingleObject(req->app->syncHandles.hEvent_LARequest, INFINITE);
	WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Write, INFINITE);
	switch(req->request.requestType){
		case LOGIN:
			CopyMemory(shm, &req->request, sizeof(LARequest));
			SetEvent(req->app->syncHandles.hEvent_LARequest_Read);

			WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Write, INFINITE);
			
			switch(shm->loginResponse){
			case LR_SUCCESS:
				req->app->loggedInTaxi.empty = false;
				_tcscpy_s(req->app->loggedInTaxi.LicensePlate, STRING_SMALL, req->request.loginRequest.licensePlate);
				req->app->loggedInTaxi.object.coordX = req->request.loginRequest.coordX;
				req->app->loggedInTaxi.object.coordY = req->request.loginRequest.coordY;
				
				return 1;
				break;
			case LR_INVALID_UNDEFINED:
				break;
			case LR_INVALID_FULL:
					break;
			case LR_INVALID_POSITION:
					break;
			}

			SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
			break;

		case ASSIGN:
			
			break;
	}

	SetEvent(req->app->syncHandles.hEvent_LARequest);
	SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
	free(req);
	return 1;
}