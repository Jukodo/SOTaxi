#pragma once

#ifdef CTDLL_EXPORTS
#define CTDLL_API __declspec(dllexport)
#else
#define CTDLL_API __declspec(dllimport)
#endif

#define NAME_MUTEX_QnARequest TEXT("JUSO2TAXI_MUTEX_QNAR")
#define NAME_MUTEX_Connecting2Central TEXT("JUSO2TAXI_MUTEX_C2C")
#define NAME_MUTEX_TossRequest TEXT("JUSO2TAXI_MUTEX_TR")
#define NAME_MUTEX_StepRoutine TEXT("JUSO2TAXI_MUTEX_SR")
#define NAME_EVENT_DestinationChanged TEXT("JUSO2TAXI_EVENT_DC")

//GenericDLL
#include "CenDLL.h"
//Data
#include "CTDLLSettings.h"
#include "CTDLLTaxi.h"
#include "CTDLLThreads.h"
#include "CTDLLService.h"