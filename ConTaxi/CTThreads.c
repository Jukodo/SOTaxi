#pragma once
#include "CTThreads.h"

DWORD WINAPI Thread_StepRoutine(LPVOID _param){
	TParam_StepRoutine* param = (TParam_StepRoutine*) _param;

	LARGE_INTEGER liTime;
	liTime.QuadPart = -10000000LL * 1 /*Triggers after 1 second*/;
	SetWaitableTimer(param->app->taxiMovementRoutine, &liTime, 1000, NULL, NULL, FALSE);
	while(param->app->keepRunning){
		WaitForSingleObject(param->app->taxiMovementRoutine, INFINITE);
		WaitForSingleObject(param->app->syncHandles.hMutex_StepRoutine, INFINITE);

		CTTaxi* loggedInTaxi = &param->app->loggedInTaxi;
		if(loggedInTaxi == NULL)
			return -401;
		
		if(param->app->loggedInTaxi.taxiInfo.state == TS_EMPTY){
			param->app->stepRoutine.currentStep = -1;
			
			if(param->app->stepRoutine.path != NULL){
				if(param->app->stepRoutine.path->path != NULL){
					free(param->app->stepRoutine.path->path);
					param->app->stepRoutine.path->path = NULL;
				}
				free(param->app->stepRoutine.path);
				param->app->stepRoutine.path = NULL;
			}
		}

		//If no path defined, walk randomly
		if(param->app->stepRoutine.path == NULL){
			Service_NewPosition(param->app, Movement_NextRandomStep(param->app, &loggedInTaxi->taxiInfo.object));
		}else{
			if(param->app->stepRoutine.path->path == NULL)
				return -401;

			param->app->stepRoutine.currentStep += param->app->loggedInTaxi.taxiInfo.object.speedMultiplier;
			if(param->app->stepRoutine.currentStep >= param->app->stepRoutine.path->steps)
				param->app->stepRoutine.currentStep = param->app->stepRoutine.path->steps;

			Service_NewPosition(param->app, param->app->stepRoutine.path->path[(int) param->app->stepRoutine.currentStep]);

			if(param->app->stepRoutine.currentStep >= param->app->stepRoutine.path->steps){
				switch(param->app->loggedInTaxi.taxiInfo.state){
					case TS_OTW_PASS:
						_tprintf(TEXT("%s[Taxi Movement] Arrived at passenger location! Picking Passenger %s up..."), 
							Utils_NewSubLine(), 
							param->app->loggedInTaxi.transportInfo.passId);
						Service_NewState(param->app, TS_WITH_PASS);
						Service_NewDestination(param->app, param->app->loggedInTaxi.transportInfo.xyDestination);
						break;
					case TS_WITH_PASS:
						_tprintf(TEXT("%s[Taxi Movement] Arrived at passenger destination! Dropping Passenger %s..."), 
							Utils_NewSubLine(),
							param->app->loggedInTaxi.transportInfo.passId);
						Service_NewState(param->app, TS_WITH_PASS);
						if(param->app->stepRoutine.path != NULL){
							if(param->app->stepRoutine.path->path != NULL){
								free(param->app->stepRoutine.path->path);
								param->app->stepRoutine.path->path = NULL;
							}
							free(param->app->stepRoutine.path);
							param->app->stepRoutine.path = NULL;
						}

						break;
				}
			}
		}
		
		ReleaseMutex(param->app->syncHandles.hMutex_StepRoutine);
	}

	free(param);
	return 401;
}

//A simple thread that received an "event" that a new destination has been set (from CTDLL Thread_NotificationReceiver_NamedPipe() on CTDLLThreads.c)
DWORD WINAPI Thread_DestinationChanger(LPVOID _param){
	TParam_DestinationChanger* param = (TParam_DestinationChanger*) _param;

	while(param->app->keepRunning){
		WaitForSingleObject(param->app->syncHandles.hEvent_DestinationChanged, INFINITE);
		Service_NewDestination(param->app, param->app->loggedInTaxi.transportInfo.xyStartingPosition);
	}

	free(param);
	return 402;
}
