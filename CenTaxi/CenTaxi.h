#pragma once
#include "CenDLL.h"

typedef struct CenTaxi CenTaxi;

struct CenTaxi{
	Taxi* taxiInfo;
	HANDLE taxiNamedPipe;
};