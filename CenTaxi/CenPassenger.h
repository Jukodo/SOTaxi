#pragma once
#include "CenDLL.h"

typedef struct CenPassenger CenPassenger;
typedef struct CPThreadHandles CPThreadHandles;

struct CPThreadHandles{
	HANDLE hTaxiAssignment;
	DWORD dwIdTaxiAssignment;
};

struct CenPassenger{
	Passenger passengerInfo;
	CPThreadHandles cpThreadHandles;
	bool* interestedTaxis;
};