#pragma once
#include "CenDLL.h"

typedef struct Application Application;
typedef struct NamedPipeHandles NamedPipeHandles;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;

typedef enum PassengerCommands PassengerCommands;

struct NamedPipeHandles{
	HANDLE hRead;
	HANDLE hWrite;
};

struct ThreadHandles{
	HANDLE hQnARequests;
	DWORD dwIdQnARequests;
};

struct Application{
	NamedPipeHandles namedPipeHandles;
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
	PC_HELP,
	PC_LOGIN,
	PC_LIST_PASSENGERS,
	PC_CLOSEAPP,
	PC_UNDEFINED
};

bool Setup_Application(Application* app);
bool Setup_OpenNamedPipeHandles(NamedPipeHandles* threadHandles);
bool Setup_OpenThreadHandles(ThreadHandles* threadHandles);
void Setup_CloseAllHandles(Application* app);

void Service_CloseApp(Application* app);
PassengerCommands Service_UseCommand(Application* app, TCHAR* command);

bool Command_LoginPassenger(Application* app, TCHAR* sId, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y);
void Command_ListPassengers(Application* app);