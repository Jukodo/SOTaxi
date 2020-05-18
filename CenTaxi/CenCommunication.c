#pragma once
#include "CenCommunication.h"

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID request){
	TParam_LARequest* req = (TParam_LARequest*) request;

	while(true){
		WaitForSingleObject(req->app->syncHandles.hEvent_QnARequest_Read, INFINITE);
		QnARequest* shm = req->app->shmHandles.lpSHM_QnARequest;

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

		SetEvent(req->app->syncHandles.hEvent_QnARequest_Write);
	}
	free(req);
	return 1;
}