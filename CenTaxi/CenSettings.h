#pragma once
#include "CenDLL.h"

#define DEFAULT_ASSIGNMENT_TIMEOUT 30
#define TOPMAX_ASSIGNMENT_TIMEOUT 300
#define DEFAULT_ALLOW_TAXI_LOGINS true

typedef struct Settings Settings;

struct Settings{
	int secAssignmentTimeout;
	bool allowTaxiLogins;
};