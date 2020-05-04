#pragma once
#include "CenDLL.h"

typedef struct Taxi Taxi;

struct Taxi{
	bool empty;
	TCHAR LicensePlate[STRING_SMALL];
	XYObject object;
};