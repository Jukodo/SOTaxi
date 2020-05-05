#pragma once
#include "CenCommunication.h"

DWORD WINAPI Thread_ReceiveLARequests(LPVOID request){
	TParam_LARequest* req = (TParam_LARequest*) request;

	while(true){
		WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Read, INFINITE);
		LARequest* shm = req->app->shmHandles.lpSHM_LARequest;

		switch(shm->requestType){
			case LOGIN:
				shm->loginResponse = Service_LoginTaxi(req->app, &shm->loginRequest);
				break;
			case ASSIGN:
				break;
		}

		SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
	}
	free(req);
	return 1;
}