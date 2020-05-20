#pragma once
#include "CTDLL.h"

#define DEFAULT_CDN 10
#define TOPMAX_CDN 50
#define DEFAULT_SPEED 1.0
#define SPEED_CHANGEBY 0.5
#define TOPMAX_SPEED 3.0
#define TOPMIN_SPEED 0.5

typedef struct Settings Settings;

struct Settings{
	int CDN; //Cell Distance Number (Taxi has to be CDN cells close to the passenger to send automatic interest)
	bool automaticInterest; //Send automatic interest requests for close passengers
	bool automaticDriving; //Drive automatically through the city
};