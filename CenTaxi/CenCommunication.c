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
			case RT_NT_INTEREST:
				shm->ntIntResponse = Service_RequestPassenger(req->app, &shm->ntIntRequest);
				break;
		}

		SetEvent(req->app->syncHandles.hEvent_QnARequest_Write);
	}
	free(req);
	return 1;
}