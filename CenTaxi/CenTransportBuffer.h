#pragma once
#include "CenDLL.h"

typedef struct TRThreadHandles TRThreadHandles;
typedef struct CenTransportRequest CenTransportRequest;

struct TRThreadHandles{
	HANDLE hTaxiAssignment;
	DWORD dwIdTaxiAssignment;
};

struct CenTransportRequest{
	TRThreadHandles cpThreadHandles;
	int* interestedTaxis;
};