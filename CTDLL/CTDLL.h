#pragma once

#ifdef CTDLL_EXPORTS
#define CTDLL_API __declspec(dllexport)
#else
#define CTDLL_API __declspec(dllimport)
#endif

//GenericDLL
#include "CenDLL.h"
//Data
#include "CTDLLService.h"
#include "CTDLLCommunication.h"