#pragma once
#include "CTDLLThreads.h"

DWORD WINAPI Thread_SendQnARequests(LPVOID _param){
	TParam_QnARequest* param = (TParam_QnARequest*) _param;
	QnARequest* shm = param->app->shmHandles.lpSHM_QnARequest;

	WaitForSingleObject(param->app->syncHandles.hMutex_QnARequest_CanAccess, INFINITE);
	WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Write, INFINITE);

	CopyMemory(shm, &param->request, sizeof(QnARequest));
	SetEvent(param->app->syncHandles.hEvent_QnARequest_Read);

	WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Write, INFINITE);

	switch(param->request.requestType){
	case QnART_LOGIN:

		switch(shm->taxiLoginResponse.taxiLoginResponseType){
			case TLR_SUCCESS:
				param->app->loggedInTaxi.taxiInfo.empty = false;
				_tcscpy_s(param->app->loggedInTaxi.taxiInfo.LicensePlate, _countof(param->app->loggedInTaxi.taxiInfo.LicensePlate), param->request.taxiLoginRequest.licensePlate);
				param->app->loggedInTaxi.taxiInfo.object.coordX = param->request.taxiLoginRequest.coordX;
				param->app->loggedInTaxi.taxiInfo.object.coordY = param->request.taxiLoginRequest.coordY;

				param->app->map.width = shm->taxiLoginResponse.mapWidth;
				param->app->map.height = shm->taxiLoginResponse.mapHeight;
				Service_GetMap(param->app);
				break;
			case TLR_INVALID_UNDEFINED:
				_tprintf(TEXT("%sError... Please try again!"), Utils_NewSubLine());
				break;
			case TLR_INVALID_CLOSED:
				_tprintf(TEXT("%sError... Central is not allowing taxi logins at the moment!"), Utils_NewSubLine());
				break;
			case TLR_INVALID_FULL:
				_tprintf(TEXT("%sError... The application doesn't accept more taxis!"), Utils_NewSubLine());
				break;
			case TLR_INVALID_POSITION:
				_tprintf(TEXT("%sError... The position chosen is invalid!"), Utils_NewSubLine());
				break;
			case TLR_INVALID_EXISTS:
				_tprintf(TEXT("%sError... The license plate chosen is already exists!"), Utils_NewSubLine());
				break;
			default:
				_tprintf(TEXT("%sOh no! This error was unexpected! Error: %d"), Utils_NewSubLine(), GetLastError());
				break;
		}

		param->request.taxiLoginResponse.taxiLoginResponseType = shm->taxiLoginResponse.taxiLoginResponseType;
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

	ReleaseMutex(param->app->syncHandles.hMutex_QnARequest_CanAccess);
	SetEvent(param->app->syncHandles.hEvent_QnARequest_Write);

	/*If the request type is LOGIN do not free param
	**param will be used back at CTService.c Service_Login() and freed after use
	*/
	if(param->request.requestType != QnART_LOGIN)
		free(param);

	return 1;
}

DWORD WINAPI Thread_NotificationReceiver_NewTransport(LPVOID _param){
	TParam_NotificationReceiver_NT* param = (TParam_NotificationReceiver_NT*) _param;
	NewTransportBuffer* buffer;

	while(param->app->keepRunning){
		WaitForSingleObject(param->app->syncHandles.hEvent_Notify_T_NewTranspReq, INFINITE);
		buffer = (NewTransportBuffer*) param->app->shmHandles.lpSHM_NTBuffer;

		while(buffer->head != param->app->NTBuffer_Tail){
			buffer->transportRequests[param->app->NTBuffer_Tail];

			Utils_DLL_Log(TEXT("ConTaxi received a notification from CenTaxi using Manual Event triggered on CenService.c (Service_NotifyTaxisNewTransport)"));

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

	TCHAR state[STRING_SMALL];
	ZeroMemory(state, STRING_SMALL*sizeof(TCHAR));
	switch(param->app->loggedInTaxi.taxiInfo.state){
		case TS_EMPTY:
			_tcscpy_s(state, _countof(state), TEXT("empty"));
			break;
		case TS_OTW_PASS:
			_tcscpy_s(state, _countof(state), TEXT("otwPass"));
			break;
		case TS_WITH_PASS:
			_tcscpy_s(state, _countof(state), TEXT("withPass"));
			break;
		case TS_STATIONARY:
			_tcscpy_s(state, _countof(state), TEXT("stationary"));
			break;
	}
	TCHAR log[STRING_XXL];
	swprintf(log, STRING_XXL, TEXT("Taxi changed position... LicensePlate: %s | X: %.2lf | Y: %.2lf | Passenger: ToDo | State: %s | Speed: %.2lf | CDN: %d"),
		param->app->loggedInTaxi.taxiInfo.LicensePlate,
		param->app->loggedInTaxi.taxiInfo.object.coordX,
		param->app->loggedInTaxi.taxiInfo.object.coordY,
		state,
		param->app->loggedInTaxi.taxiInfo.object.speedMultiplier,
		param->app->settings.CDN);
	Utils_DLL_Log(log);

	ReleaseSemaphore(param->app->syncHandles.hSemaphore_HasTossRequest, 1, NULL);
	ReleaseMutex(param->app->syncHandles.hMutex_TossRequest_CanAccess);

	free(param);
	return 1;
}

DWORD WINAPI Thread_NotificationReceiver_NamedPipe(LPVOID _param){
	TParam_NotificationReceiver_NP* param = (TParam_NotificationReceiver_NP*) _param;

	CommsTC notificationReceived;
	while(param->app->keepRunning){
		ReadFile(
			param->app->loggedInTaxi.centralNamedPipe,	//Named pipe handle
			&notificationReceived,						//Read into
			sizeof(CommsTC),							//Size being read
			NULL,										//Quantity of bytes read
			NULL);										//Overlapped IO

		switch(notificationReceived.commType){
			case CTC_ASSIGNED:
				_tprintf(TEXT("%s[Remove Me] I've received a message that i've been assigned to [%s] at (%.2lf, %.2lf)!"), Utils_NewLine(), notificationReceived.assignComm.passId, notificationReceived.assignComm.coordX, notificationReceived.assignComm.coordY);
				Service_SetNewDestination(param->app, notificationReceived.assignComm.coordX, notificationReceived.assignComm.coordY);
				break;
			case CTC_SHUTDOWN:
				_tprintf(TEXT("%sI've been ordered to shutdown!"), Utils_NewLine());
				switch(notificationReceived.shutdownComm.shutdownType){
					case ST_GLOBAL:
						_tprintf(TEXT("%sThe shutdown was global!"), Utils_NewSubLine());
						break;
					case ST_KICKED:
						_tprintf(TEXT("%sI've been kicked!"), Utils_NewSubLine());
						break;
				}
				_tprintf(TEXT("%sReason: %s"), Utils_NewSubLine(), notificationReceived.shutdownComm.message);

				param->app->keepRunning = false;
				Utils_CloseNamedPipe(param->app->loggedInTaxi.centralNamedPipe);
				if(param->app->taxiMovementRoutine != NULL)
					CancelWaitableTimer(param->app->taxiMovementRoutine);

				break;
		}
	}

	free(param);
	return 1;
}