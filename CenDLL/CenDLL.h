#pragma once

#ifdef CENDLL_EXPORTS
#define CENDLL_API __declspec(dllexport)
#else
#define CENDLL_API __declspec(dllimport)
#endif

#if _WIN32 || _WIN64
#if _WIN64
#define DLLPATH TEXT("../ImplicitDLL/SO2_TP_DLL_64.dll")
#else
#define DLLPATH TEXT("../ImplicitDLL/SO2_TP_DLL_32.dll")
#endif
#endif

#define ALLOW_PROF_DLL true

#define MAX_NEIGHBORS 4
#define DEFAULT_SPEED 1.0

#define NTBUFFER_MAX 5
#define TOSSBUFFER_MAX 20
#define TOPMAX_TAXI 50
#define TOPMAX_PASSENGERS 50

#define SECOND_IN_MILI 1000

#define STRING_SMALL 15
#define STRING_MEDIUM 25
#define STRING_LARGE 50
#define STRING_XXL 255

#define STRING_LICENSEPLATE 9
#define STRING_ID 9

#define SHUTDOWN_REASON_Undefined TEXT("Undefined Reason")
#define SHUTDOWN_REASON_Global TEXT("Application is closing")

#define NAME_SHM_TaxiList TEXT("JUSO2TAXI_SHM_TL")
#define NAME_SHM_PassengerList TEXT("JUSO2TAXI_SHM_PL")
#define NAME_SHM_QnARequest TEXT("JUSO2TAXI_SMH_QNAR")
#define NAME_SHM_TransportRequestBuffer TEXT("JUSO2TAXI_SMH_TRB")
#define NAME_SHM_Map TEXT("JUSO2TAXI_SMH_MAP")
#define NAME_SHM_TossRequestBuffer TEXT("JUSO2TAXI_SMH_TOSSRB")

#define NAME_EVENT_TaxiList TEXT("JUSO2TAXI_EVENT_TL")
#define NAME_EVENT_PassengerList TEXT("JUSO2TAXI_EVENT_PL")
#define NAME_EVENT_QnARequest_Read TEXT("JUSO2TAXI_EVENT_QNAR_R")
#define NAME_EVENT_QnARequest_Write TEXT("JUSO2TAXI_EVENT_QNAR_W")
#define NAME_EVENT_NewTransportRequest TEXT("JUSO2TAXI_EVENT_NTR")
#define NAME_EVENT_NewTaxiSpot TEXT("JUSO2TAXI_EVENT_NTS")

#define NAME_SEMAPHORE_HasTossRequest TEXT("JUSO2TAXI_SEMAPHORE_HTR")
#define NAME_SEMAPHORE_TaxiNPSpots TEXT("JUSO2TAXI_SEMAPHORE_TNPS")

#define NAME_NAMEDPIPE_CommsC2T TEXT("\\\\.\\pipe\\JUSO2TAXI_NAMEDPIPE_CC2T")
#define NAME_NAMEDPIPE_CommsP2C TEXT("\\\\.\\pipe\\JUSO2TAXI_NAMEDPIPE_CP2C")
#define NAME_NAMEDPIPE_CommsC2P TEXT("\\\\.\\pipe\\JUSO2TAXI_NAMEDPIPE_CC2P")
#define NAME_NAMEDPIPE_CommsPCQnA TEXT("\\\\.\\pipe\\JUSO2TAXI_NAMEDPIPE_CPCQnA")

//Main
#include <stdio.h>
//Bool
#include <stdbool.h>
//SRand time
#include <time.h>
//Ceil
#include <math.h>
#include <stdlib.h>
//Unicode
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
//Data
#include "DLLPath.h"
#include "DLLNodeList.h"
#include "DLLMap.h"
#include "DLLTaxi.h"
#include "DLLUtils.h"
#include "DLLPassenger.h"
#include "DLLCommunication.h"

HINSTANCE hLib;