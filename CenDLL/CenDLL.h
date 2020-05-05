#pragma once

#ifdef CENDLL_EXPORTS
#define CENDLL_API __declspec(dllexport)
#else
#define CENDLL_API __declspec(dllimport)
#endif

#define STRING_SMALL 15
#define STRING_MEDIUM 25
#define STRING_LARGE 50
#define STRING_XXL 255

#define SHM_Testing TEXT("justTestingKek")
#define NAME_ SHM_SendRequest TEXT("JUSO2TAXI_SR")

#define NAME_MUTEX_ONEINSTANCE_CEN TEXT("JUSO2TAXI_ONEINSTANCE_CEN")
#define NAME_MUTEX_ONEINSTANCE_CP TEXT("JUSO2TAXI_ONEINSTANCE_CP")

#define NAME_EVENT_LARequest TEXT("JUSO2TAXI_LAR")
#define NAME_EVENT_LARequest_Read TEXT("JUSO2TAXI_LARW")
#define NAME_EVENT_LARequest_Write TEXT("JUSO2TAXI_LARR")

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
