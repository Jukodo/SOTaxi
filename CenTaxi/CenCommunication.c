#pragma once
#include "CenCommunication.h"

DWORD WINAPI Thread_ReceiveLARequests(LPVOID request){
	TParam_LARequest* req = (TParam_LARequest*) request;

	while(true){
		WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Read, INFINITE);
		LARequest* shm = req->app->shmHandles.lpSHM_LARequest;

		switch(shm->requestType){
			case RT_LOGIN:
				shm->loginResponse = Service_LoginTaxi(req->app, &shm->loginRequest);
				break;
			case RT_ASSIGN:
				shm->assignResponse = Service_RequestPassenger(req->app, &shm->assignRequest);
				break;
			case RT_CDN:
				break;
			case RT_VAR:
				shm->varResponse.maxTaxis = req->app->maxTaxis;
				shm->varResponse.maxPassengers = req->app->maxPassengers;
				break;
		}

		SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
	}
	free(req);
	return 1;
}