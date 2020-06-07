#pragma once

#ifdef CENDLL_EXPORTS
#define CENDLL_API __declspec(dllexport)
#else
#define CENDLL_API __declspec(dllimport)
#endif

#define NTBUFFER_MAX 5
#define TOSSBUFFER_MAX 20
#define TOPMAX_TAXI 50
#define TOPMAX_PASSENGERS 50

#define STRING_SMALL 15
#define STRING_MEDIUM 25
#define STRING_LARGE 50
#define STRING_XXL 255

#define STRING_LICENSEPLATE 9

#define SHUTDOWN_REASON_Undefined TEXT("Undefined Reason")
#define SHUTDOWN_REASON_Global TEXT("Application is closing")

#define NAME_SHM_QnARequest TEXT("JUSO2TAXI_SMH_QNAR")
#define NAME_SHM_TransportRequestBuffer TEXT("JUSO2TAXI_SMH_TRB")
#define NAME_SHM_Map TEXT("JUSO2TAXI_SMH_MAP")
#define NAME_SHM_TossRequestBuffer TEXT("JUSO2TAXI_SMH_TOSSRB")

#define NAME_MUTEX_OneInstance_CEN TEXT("JUSO2TAXI_MUTEX_OI_CEN")
#define NAME_MUTEX_OneInstance_CP TEXT("JUSO2TAXI_OI_CP")

#define NAME_EVENT_QnARequest_Read TEXT("JUSO2TAXI_EVENT_QNAR_R")
#define NAME_EVENT_QnARequest_Write TEXT("JUSO2TAXI_EVENT_QNAR_W")
#define NAME_EVENT_NewTransportRequest TEXT("JUSO2TAXI_EVENT_NTR")
#define NAME_EVENT_TaxiLoggingIn TEXT("JUSO2TAXI_EVENT_TLI")

#define NAME_SEMAPHORE_HasTossRequest TEXT("JUSO2TAXI_SEMAPHORE_HTR")

#define NAME_NAMEDPIPE_CommsTaxiCentral TEXT("\\\\.\\pipe\\JUSO2TAXI_NAMEDPIPE_CTC")

//Main
#include <stdio.h>
//Bool
#include <stdbool.h>
//SRand time
#include <time.h>
//Unicode
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
//Data
#include "DLLMap.h"
#include "DLLTaxi.h"
#include "DLLUtils.h"
#include "DLLPassenger.h"
#include "DLLCommunication.h"