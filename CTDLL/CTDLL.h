#pragma once

#ifdef CTDLL_EXPORTS
#define CTDLL_API __declspec(dllexport)
#else
#define CTDLL_API __declspec(dllimport)
#endif

#define NAME_MUTEX_QnARequest TEXT("JUSO2TAXI_MUTEX_QNAR")
#define NAME_MUTEX_TossRequest TEXT("JUSO2TAXI_MUTEX_TR")

//GenericDLL
#include "CenDLL.h"
//Data
#include "CTDLLSettings.h"
#include "CTDLLService.h"
#include "CTDLLCommunication.h"