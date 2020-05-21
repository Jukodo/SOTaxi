#pragma once
#include "CenCommunication.h"

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID _param){
	TParam_QnARequest* param = (TParam_QnARequest*) _param;

	while(true){
		WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Read, INFINITE);
		QnARequest* shm = param->app->shmHandles.lpSHM_QnARequest;

		switch(shm->requestType){
			case RT_LOGIN:
				shm->loginResponse.loginResponseType = Service_LoginTaxi(param->app, &shm->loginRequest);

				shm->loginResponse.mapWidth = param->app->map.width;
				shm->loginResponse.mapHeight = param->app->map.height;
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