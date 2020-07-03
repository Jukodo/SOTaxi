#pragma once
#include "CenDLL.h"

typedef enum TaxiState TaxiState;
typedef struct TransportInfo TransportInfo;
typedef struct Taxi Taxi;

struct TransportInfo{
	TCHAR passId[STRING_ID];
	XY xyStartingPosition;
	XY xyDestination;
};

enum TaxiState{
	TS_EMPTY, //Is empty
	TS_OTW_PASS, //On The Way to the passenger
	TS_WITH_PASS, //Currently transporting a passenger
	TS_STATIONARY //Currently transporting a passenger
};

struct Taxi{
	bool empty;
	TCHAR LicensePlate[STRING_SMALL];
	XYObject object;
	TaxiState state;
	TransportInfo transporting;
};