#pragma once
#include "CPService.h"

typedef struct CPPassenger CPPassenger;

struct CPPassenger{
	Passenger passengerInfo;
	XY xyDestination;
};