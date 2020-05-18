#pragma once

#ifdef CENDLL_EXPORTS
#define CENDLL_API __declspec(dllexport)
#else
#define CENDLL_API __declspec(dllimport)
#endif

#define NTBUFFER_MAX 5
#define TOPMAX_TAXI 50
#define TOPMAX_PASSENGERS 50

#define STRING_SMALL 15
#define STRING_MEDIUM 25
#define STRING_LARGE 50
#define STRING_XXL 255

#define NAME_SHM_LARequests TEXT("JUSO2TAXI_LA")
#define NAME_SHM_TransportRequestBuffer TEXT("JUSO2TAXI_CPBUFFER")

#define NAME_MUTEX_OneInstance_CEN TEXT("JUSO2TAXI_ONEINSTANCE_CEN")
#define NAME_MUTEX_OneInstance_CP TEXT("JUSO2TAXI_ONEINSTANCE_CP")
#define NAME_MUTEX_LARequest TEXT("JUSO2TAXI_LAR")

#define NAME_EVENT_LARequest_Read TEXT("JUSO2TAXI_LARW")
#define NAME_EVENT_LARequest_Write TEXT("JUSO2TAXI_LARR")
#define NAME_EVENT_NewTransportRequest TEXT("JUSO2TAXI_NEWTPR")

//Main
#include <stdio.h>
//Bool
#include <stdbool.h>
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