#pragma once
#include "CenCommunication.h"

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID _param){
	TParam_QnARequest* param = (TParam_QnARequest*) _param;

	while(true){
		WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Read, INFINITE);
		QnARequest* shm = param->app->shmHandles.lpSHM_QnARequest;

		switch(shm->requestType){
			case QnART_LOGIN:
				shm->loginResponse.loginResponseType = Service_LoginTaxi(param->app, &shm->loginRequest);

				shm->loginResponse.mapWidth = param->app->map.width;
				shm->loginResponse.mapHeight = param->app->map.height;
				break;
			case QnART_NT_INTEREST:
				shm->ntIntResponse = Service_RegisterInterest(param->app, &shm->ntIntRequest);
				break;
		}

		SetEvent(param->app->syncHandles.hEvent_QnARequest_Write);
	}
	free(param);
	return 1;
}

DWORD WINAPI Thread_TaxiAssignment(LPVOID _param){
	TParam_TaxiAssignment* param = (TParam_TaxiAssignment*) _param;
	CenPassenger* myPassenger = Get_Passenger(param->app, param->myIndex);

	HANDLE hAssignTimeout = CreateWaitableTimer(
		NULL,	//Security Attributes
		TRUE,	//Manual Reset
		NULL	//WaitableTimer name
	);
	if(hAssignTimeout == NULL)
		return -1;

	LARGE_INTEGER liTime;
	liTime.QuadPart = -10000000LL * param->app->settings.secAssignmentTimeout;

	SetWaitableTimer(
		hAssignTimeout,
		&liTime,
		0,
		NULL,
		NULL,
		FALSE
	);

	WaitForSingleObject(hAssignTimeout, INFINITE);

	int numIntTaxis = 0;
	for(int i = 0; i < param->app->maxTaxis; i++){
		if(myPassenger->interestedTaxis[i] != -1){
			numIntTaxis++;
		}
	}

	if(numIntTaxis == 0){//No interested taxis
		_tprintf(TEXT("%sNo taxi has shown interest towards %s!"), Utils_NewSubLine(), myPassenger->passengerInfo.Id);
		return 1;
	}

	_tprintf(TEXT("%sQuantity of interested taxis = %d"), Utils_NewSubLine(), numIntTaxis);
	int a = (rand() % numIntTaxis);
	_tprintf(TEXT("%sChosen taxi is %s"), Utils_NewSubLine(), Get_Taxi(param->app, myPassenger->interestedTaxis[a])->LicensePlate);
	/*TAG_TODO
	**Notify Taxi that he has been chosen and assigned to respective passenger
	**Notify Passenger that he has be assigned to respective taxi
	*/

	free(param);
	return 1;
}

DWORD WINAPI Thread_ConsumeTossRequests(LPVOID _param){
	TParam_ConsumeTossRequests* param = (TParam_ConsumeTossRequests*) _param;
	TossRequestsBuffer* buffer = (TossRequestsBuffer*) param->app->shmHandles.lpSHM_TossReqBuffer;

	while(true){
		WaitForSingleObject(param->app->syncHandles.hSemaphore_HasTossRequest, INFINITE);
		if(buffer->tail != buffer->head){
			buffer->tossRequests[buffer->tail];
			_tprintf(TEXT("%sCentral consumed a toss request... TossType: %d"), Utils_NewLine(), buffer->tossRequests[buffer->tail].tossType);
			switch(buffer->tossRequests[buffer->tail].tossType){
				case TRT_TAXI_POSITION:
					_tprintf(TEXT("%sAnd it is a taxi position toss!"), Utils_NewSubLine());
					_tprintf(TEXT("%sNew position of taxi %s is X:%.2lf Y:%.2lf..."), 
						Utils_NewSubLine(),
						buffer->tossRequests[buffer->tail].tossPosition.licensePlate,
						buffer->tossRequests[buffer->tail].tossPosition.newX,
						buffer->tossRequests[buffer->tail].tossPosition.newY);
					break;
				case TRT_TAXI_STATE:
					_tprintf(TEXT("%sAnd it is a taxi state toss!"), Utils_NewSubLine());
					TCHAR state[STRING_MEDIUM];
					switch(buffer->tossRequests[buffer->tail].tossState.newState){
						case TS_EMPTY:
							_tcscpy_s(state, _countof(state), TEXT("Empty"));
							break;
						case TS_OTW_PASS:
							_tcscpy_s(state, _countof(state), TEXT("PickingUpPassenger"));
							break;
						case TS_WITH_PASS:
							_tcscpy_s(state, _countof(state), TEXT("WithPassenger"));
							break;
						case TS_STATIONARY:
							_tcscpy_s(state, _countof(state), TEXT("Stationary"));
							break;
						default:
							return -1;
					}
					_tprintf(TEXT("%sNew state of taxi %s is %s..."),
						Utils_NewSubLine(),
						buffer->tossRequests[buffer->tail].tossState.licensePlate,
						state);
					break;
			}
			buffer->tail = (buffer->tail + 1) % TOSSBUFFER_MAX;
		}
	}

	free(param);
	return 1;
}