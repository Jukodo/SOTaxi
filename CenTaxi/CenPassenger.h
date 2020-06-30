#pragma once
#include "CenDLL.h"

typedef struct CenPassenger CenPassenger;

struct CenPassenger{
	Passenger* passengerInfo;
	XY xyDestination;
};