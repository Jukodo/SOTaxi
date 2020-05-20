#pragma once
#include "CenCommunication.h"

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID _param){
	TParam_QnARequest* param = (TParam_QnARequest*) _param;

	while(true){
		WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Read, INFINITE);
		QnARequest* shm = param->app->shmHandles.lpSHM_QnARequest;

		switch(shm->requestType){
			case RT_LOGIN:
				shm->loginResponse = Service_LoginTaxi(param->app, &shm->loginRequest);
				break;
			case RT_NT_INTEREST:
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
	liTime.QuadPart = -10000000LL * /*TAG_TODO*/10;

	SetWaitableTimer(
		hAssignTimeout,
		&liTime,
		0,
		NULL,
		NULL,
		FALSE
	);

	WaitForSingleObject(hAssignTimeout, INFINITE);
	_tprintf(TEXT("\n%s thread is alive!"), myPassenger->passengerInfo.Id);
	for(int i = 0; i < param->app->maxTaxis; i++){
		if(myPassenger->interestedTaxis[i])
			_tprintf(TEXT("\nTaxi %s is interested!"), Get_Taxi(param->app, i)->LicensePlate);
	}

	free(param);
	return 1;
}