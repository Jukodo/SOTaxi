#pragma once
#include "CenDLL.h"

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;

typedef enum PassengerCommands PassengerCommands;

struct ThreadHandles{
	HANDLE hQnARequests;
	DWORD dwIdQnARequests;
};

struct Application{
	ThreadHandles threadHandles;
	bool keepRunning; //Used to identify if app should keep running, when FALSE threads that are infinitely looping will end
};

#define CMD_HELP TEXT("/help")
#define CMD_LOGIN TEXT("/login")
#define CMD_LIST_PASSENGERS TEXT("/listPassengers")
#define CMD_CLOSEAPP TEXT("/closeApp")

#define TIMEOUT_TaxiLoginQueue_Seconds 30 //30 seconds
#define TIMEOUT_TaxiLoginQueue SECOND_IN_MILI * TIMEOUT_TaxiLoginQueue_Seconds //30 seconds

enum PassengerCommands{
	TC_HELP,
	TC_LOGIN,
	TC_LIST_PASSENGERS,
	TC_CLOSEAPP,
	TC_UNDEFINED
};