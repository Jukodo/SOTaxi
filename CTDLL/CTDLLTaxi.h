#pragma once
#include "CTDLL.h"

typedef struct CTTaxi CTTaxi;

struct CTTaxi{
	Taxi taxiInfo;
	HANDLE centralNamedPipe;
};