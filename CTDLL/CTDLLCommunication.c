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
	case QnART_LOGIN:

		switch(shm->loginResponse.loginResponseType){
			case LR_SUCCESS:
				param->app->loggedInTaxi.empty = false;
				_tcscpy_s(param->app->loggedInTaxi.LicensePlate, _countof(param->app->loggedInTaxi.LicensePlate), param->request.loginRequest.licensePlate);
				param->app->loggedInTaxi.object.coordX = param->request.loginRequest.coordX;
				param->app->loggedInTaxi.object.coordY = param->request.loginRequest.coordY;

				param->app->map.width = shm->loginResponse.mapWidth;
				param->app->map.height = shm->loginResponse.mapHeight;
				Service_GetMap(param->app);
				break;
			case LR_INVALID_UNDEFINED:
				_tprintf(TEXT("%sError... Please try again!"), Utils_NewSubLine());
				break;
			case LR_INVALID_CLOSED:
				_tprintf(TEXT("%sError... Central is not allowing taxi logins at the moment!"), Utils_NewSubLine());
				break;
			case LR_INVALID_FULL:
				_tprintf(TEXT("%sError... The application doesn't accept more taxis!"), Utils_NewSubLine());
				break;
			case LR_INVALID_POSITION:
				_tprintf(TEXT("%sError... The position chosen is invalid!"), Utils_NewSubLine());
				break;
			case LR_INVALID_EXISTS:
				_tprintf(TEXT("%sError... The license plate chosen is already exists!"), Utils_NewSubLine());
				break;
			default:
				_tprintf(TEXT("%sOh no! This error was unexpected! Error: %d"), Utils_NewSubLine(), GetLastError());
				break;
		}

		break;

	case QnART_NT_INTEREST:

		switch(shm->ntIntResponse){
			case NTIR_SUCCESS:
				_tprintf(TEXT("%sYour interest towards transporting Passenger[%s] has been registered!"), Utils_NewSubLine(), shm->ntIntRequest.idPassenger);
				break;
			case NTIR_INVALID_UNDEFINED:
				_tprintf(TEXT("%sError! Please try again..."), Utils_NewSubLine());
				break;
			case NTIR_INVALID_CLOSED:
				_tprintf(TEXT("%sError! This transport has already been closed..."), Utils_NewSubLine());
				break;
			case NTIR_INVALID_ID:
				_tprintf(TEXT("%sError! There is no passenger with ID = %s..."), Utils_NewSubLine(), shm->ntIntRequest.idPassenger);
				break;
			default:
				_tprintf(TEXT("%sOh no! This error was unexpected! Error: %d"), Utils_NewSubLine(), GetLastError());
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

			_tprintf(TEXT("%sA new transport request has been submited!%s%s is waiting at (%.2f, %.2f) for a taxi!%sPlease use \"%s\" if you are interested!%s"), 
				Utils_NewSubLine(), 
				Utils_NewSubLine(),
				buffer->transportRequests[param->app->NTBuffer_Tail].Id,
				buffer->transportRequests[param->app->NTBuffer_Tail].object.coordX,
				buffer->transportRequests[param->app->NTBuffer_Tail].object.coordY,
				Utils_NewSubLine(),
				CMD_REQUEST_INTEREST,
				Utils_NewSubLine());

			param->app->NTBuffer_Tail = (param->app->NTBuffer_Tail + 1) % NTBUFFER_MAX;
		}
	}

	free(param);
	return 1;
}

DWORD WINAPI Thread_TossRequest(LPVOID _param){
	TParam_TossRequest* param = (TParam_TossRequest*) _param;
	TossRequestsBuffer* buffer = (TossRequestsBuffer*) param->app->shmHandles.lpSHM_TossReqBuffer;

	WaitForSingleObject(param->app->syncHandles.hMutex_TossRequest_CanAccess, INFINITE);
	
	buffer->tossRequests[buffer->head] = param->tossRequest;
	buffer->head = (buffer->head + 1) % TOSSBUFFER_MAX;

	ReleaseSemaphore(param->app->syncHandles.hSemaphore_HasTossRequest, 1, NULL);
	ReleaseMutex(param->app->syncHandles.hMutex_TossRequest_CanAccess);

	free(param);
	return 1;
}