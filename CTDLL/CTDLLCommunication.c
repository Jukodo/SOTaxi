#pragma once
#include "CTDLLCommunication.h"

DWORD WINAPI Thread_SendQnARequests(LPVOID _param){
	TParam_QnARequest* param = (TParam_QnARequest*) _param;
	QnARequest* shm = param->app->shmHandles.lpSHM_QnARequest;

	WaitForSingleObject(param->app->syncHandles.hMutex_QnARequest, INFINITE);
	WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Write, INFINITE);

	CopyMemory(shm, &param->request, sizeof(QnARequest));
	SetEvent(param->app->syncHandles.hEvent_QnARequest_Read);

	WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Write, INFINITE);

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

	case RT_NT_INTEREST:

		switch(shm->ntIntResponse){
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

	ReleaseMutex(param->app->syncHandles.hMutex_QnARequest);
	SetEvent(param->app->syncHandles.hEvent_QnARequest_Write);
	free(param);
	return 1;
}

DWORD WINAPI Thread_NotificationReceiver_NewTransport(LPVOID _param){
	TParam_NotificationReceiver_NT* param = (TParam_NotificationReceiver_NT*) _param;
	NewTransportBuffer* buffer;

	while(true){
		WaitForSingleObject(param->app->syncHandles.hEvent_Notify_T_NewTranspReq, INFINITE);
		buffer = (NewTransportBuffer*) param->app->shmHandles.lpSHM_NTBuffer;

		while(buffer->head != param->app->NTBuffer_Tail){
			buffer->transportRequests[param->app->NTBuffer_Tail];

			_tprintf(TEXT("%sA new transport request has been submited!%s%s is waiting at (%.2f, %.2f) for a taxi!%sPlease use \"/requestPass\" if you are interested!%s"), 
				Utils_NewSubLine(), 
				Utils_NewSubLine(),
				buffer->transportRequests[param->app->NTBuffer_Tail].Id,
				buffer->transportRequests[param->app->NTBuffer_Tail].object.coordX,
				buffer->transportRequests[param->app->NTBuffer_Tail].object.coordY,
				Utils_NewSubLine(),
				Utils_NewSubLine());

			param->app->NTBuffer_Tail = (param->app->NTBuffer_Tail + 1) % NTBUFFER_MAX;
		}
	}
}