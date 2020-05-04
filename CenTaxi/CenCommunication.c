#pragma once
#include "CenCommunication.h"

DWORD WINAPI Thread_ReceiveLARequests(LPVOID request){
	TParam_LARequest* req = (TParam_LARequest*) request;

	while(true){
		_tprintf(TEXT("%sCen is starting the process"), Utils_NewSubLine());
		Sleep(1 * 1000);
		WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Read, INFINITE);
		_tprintf(TEXT("%sCen is reading shared memory and adding response"), Utils_NewSubLine());
		Sleep(1 * 1000);
		LARequest* receivedRequest = (LARequest*) req->app->shmHandles.lpSHM_LARequest;
		SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
		_tprintf(TEXT("%sCen is ending the process"), Utils_NewSubLine());
		Sleep(1 * 1000);
	}

	free(req);
	return 1;
}