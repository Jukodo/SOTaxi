#pragma once
#include "CTCommunication.h"

DWORD WINAPI Thread_SendLARequests(LPVOID request){
	TParam_LARequest* req = (TParam_LARequest*) request;

	_tprintf(TEXT("%sCT is starting the process"), Utils_NewSubLine());
	Sleep(1 * 1000);
	WaitForSingleObject(req->app->syncHandles.hEvent_LARequest, INFINITE);
	WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Write, INFINITE);
	switch(req->request.requestType){
		case LOGIN:
			//CopyMemory((LPVOID) req->app->shmHandles.lpSHM_LARequest, &req->request, sizeof(LARequest));
			_tprintf(TEXT("%sCT is writting into the shared memory"), Utils_NewSubLine());
			Sleep(1 * 1000);
			SetEvent(req->app->syncHandles.hEvent_LARequest_Read);
			WaitForSingleObject(req->app->syncHandles.hEvent_LARequest_Write, INFINITE);
			_tprintf(TEXT("%sCT has received the response and is going to finish the process"), Utils_NewSubLine());
			Sleep(1 * 1000);
			SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
			break;

		case ASSIGN:
			
			break;
	}

	SetEvent(req->app->syncHandles.hEvent_LARequest);
	SetEvent(req->app->syncHandles.hEvent_LARequest_Write);
	_tprintf(TEXT("%sCT has finished"), Utils_NewSubLine());
	Sleep(1 * 1000);
	free(req);
	return 1;
}