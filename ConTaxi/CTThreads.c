#pragma once
#include "CTThreads.h"

DWORD WINAPI Thread_StepRoutine(LPVOID _param){
	TParam_StepRoutine* param = (TParam_StepRoutine*) _param;

	LARGE_INTEGER liTime;
	liTime.QuadPart = -10000000LL * 1 /*Triggers after 1 second*/;
	SetWaitableTimer(param->app->taxiMovementRoutine, &liTime, 1000, NULL, NULL, FALSE);
	while(param->app->keepRunning){
		WaitForSingleObject(param->app->taxiMovementRoutine, INFINITE);

		CTTaxi* loggedInTaxi = &param->app->loggedInTaxi;
		if(param->app->loggedInTaxi.taxiInfo.state == TS_EMPTY){
			if(Movement_NextRandomStep(param->app, &loggedInTaxi->taxiInfo.object)){
				Service_NewPosition(param->app, loggedInTaxi->taxiInfo.object.xyPosition);
			} else{
				CancelWaitableTimer(param->app->taxiMovementRoutine);
			}
		}
	}

	free(param);
	return 1;
}