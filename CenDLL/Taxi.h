#pragma once
#include "CenDLL.h"

typedef struct Taxi{
	bool deleted;
	int LicensePlate;
	int IdPassenger;
}Taxi;

CENDLL_API int TAXI_A();
