#pragma once
#include "CenDLL.h"

typedef struct Taxi Taxi;
typedef enum TaxiStatus TaxiStatus;

struct Taxi{
	bool empty;
	TCHAR LicensePlate[STRING_SMALL];
	XYObject object;
};

enum TaxiStatus{
	TS_EMPTY, //Is empty
	TS_OTW_PASS, //On The Way to the passenger
	TS_WITH_PASS, //Currently transporting a passenger
	TS_STATIONARY //Currently transporting a passenger
};