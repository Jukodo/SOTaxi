#pragma once
#include "CenDLL.h"

typedef struct Passenger Passenger;

struct Passenger{
	bool empty;
	TCHAR Id[STRING_SMALL];
	XYObject object;
};