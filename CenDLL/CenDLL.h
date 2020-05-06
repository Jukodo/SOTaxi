#pragma once

#ifdef CENDLL_EXPORTS
#define CENDLL_API __declspec(dllexport)
#else
#define CENDLL_API __declspec(dllimport)
#endif

#define TOPMAX_TAXI 50
#define TOPMAX_PASSENGERS 50

#define STRING_SMALL 15
#define STRING_MEDIUM 25
#define STRING_LARGE 50
#define STRING_XXL 255

#define NAME_SHM_LAREQUESTS TEXT("JUSO2TAXI_LA")
#define NAME_SHM_PASSLIST TEXT("JUSO2TAXI_PASSLIST")

#define NAME_MUTEX_ONEINSTANCE_CEN TEXT("JUSO2TAXI_ONEINSTANCE_CEN")
#define NAME_MUTEX_ONEINSTANCE_CP TEXT("JUSO2TAXI_ONEINSTANCE_CP")

#define NAME_MUTEX_LARequest TEXT("JUSO2TAXI_LAR")
#define NAME_EVENT_LARequest_Read TEXT("JUSO2TAXI_LARW")
#define NAME_EVENT_LARequest_Write TEXT("JUSO2TAXI_LARR")
#define NAME_EVENT_PassengerList_Access TEXT("JUSO2TAXI_PASSLIST_ACCESS")

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
#include "DLLCommunication.h"
#include "DLLPassenger.h"
