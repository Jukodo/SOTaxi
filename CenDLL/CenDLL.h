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

#define NAME_MUTEX_RequestsTaxiToCentral TEXT("JUSO2TAXI_RT2C")

//Main
#include <stdio.h>
//Bool
#include <stdbool.h>
//Unicode
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
//Data
#include "Map.h"
#include "Taxi.h"
#include "Utils.h"
#include "Communication.h"
